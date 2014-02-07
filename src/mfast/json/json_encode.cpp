#include "json.h"
#include <boost/io/ios_state.hpp>

namespace mfast {
  namespace json {
    namespace encode_detail {

      struct quoted_string {
        quoted_string(const char* str)
          : str_(str)
        {
        }

        const char* str_;
      };


      std::ostream& operator << (std::ostream& os, quoted_string str)
      {
        os.put('"');
        const char* ptr = str.str_;
        while (*ptr != '\x0') {
          if (*ptr == '\\' || *ptr == '"')
            os.put('\\');
          os.put(*ptr++);
        }
        os.put('"');
        return os;
      }

      class json_visitor
      {
      private:
        std::ostream& strm_;
        char separator_[2];

      public:

        enum {
          visit_absent = 0
        };

        json_visitor(std::ostream& strm)
          : strm_(strm)
        {
          separator_[0] = 0;
          separator_[1] = 0;
        }

        template <typename NumericTypeRef>
        void visit(const NumericTypeRef& ref)
        {
          strm_ <<  separator_ << ref.value();
        }

        void visit(const enum_cref& ref)
        {
          if (ref.is_boolean())
            strm_ <<  separator_ << ref.value_name();
          else
            strm_ << separator_ << ref.value();
        }

        template <typename Char>
        void visit(const mfast::string_cref<Char>& ref)
        {
          strm_ <<  separator_ << quoted_string(ref.c_str());
        }

        void visit(const mfast::byte_vector_cref& ref)
        { // json doesn't have byte vector, treat it as string now
          strm_ <<  separator_ << "\"";
          boost::io::ios_flags_saver  ifs( strm_ );
          strm_  << std::hex << std::setfill('0') << std::setw(2);

          for (std::size_t i = 0; i < ref.size(); ++i)
          {
            // if the size is 16, we treat it as a UUID
            if (ref.size() == 16 && (i==4 || i==6 || i==8 || i==10))
              strm_ << '-';
            strm_   <<  (0xFF & (int) ref[i]) ;
          }
          strm_ << "\"" << std::setfill(' ');
        }

        template <typename IntType>
        void visit(const mfast::int_vector_cref<IntType>& ref)
        {
          strm_ << separator_  << "[";
          separator_[0] = '\0';

          for (std::size_t i = 0; i < ref.size(); ++i) {
            strm_ << separator_ << ref[i];
            separator_[0] = ',';
          }
          strm_ << "]";
        }

        void visit(const mfast::aggregate_cref& ref, int)
        {
          if (ref.num_fields()  == 1) {
            field_cref f0 = ref[0];
            if (f0.instruction()->field_type() == mfast::field_type_templateref) {
              if (f0.present())
                this->visit(mfast::nested_message_cref(f0),0);
              return;
            }
          }

          strm_ << separator_ <<  "{";
          separator_[0] = '\0';

          for (std::size_t i = 0; i < ref.num_fields(); ++i) {
            if (ref[i].present()) {
              strm_ << separator_ << quoted_string(ref[i].name()) << ":";
              separator_[0] = '\0';
              ref[i].accept_accessor(*this);
            }
            separator_[0] = ',';
          }

          strm_ << "}";
        }

        void visit(const mfast::sequence_cref& ref, int)
        {
          strm_ << separator_  << "[";
          if (ref.size()) {
            separator_[0] = '\0';
            ref.accept_accessor(*this);
          }
          strm_ << "]";
        }

        void visit(const mfast::sequence_element_cref& ref, int)
        {
          if (ref.element_unnamed()) {
            ref[0].accept_accessor(*this);
          }
          else {
            this->visit(mfast::aggregate_cref(ref), 0);
          }
          separator_[0] = ',';
        }

        void visit(const mfast::nested_message_cref& ref, int)
        {
          this->visit(mfast::aggregate_cref(ref), 0);
        }

      };

    } // namspace encode_detail

    bool encode(std::ostream& os, const mfast::aggregate_cref& msg)
    {
      encode_detail::json_visitor visitor(os);
      visitor.visit(msg, 0);
      return os.good();
    }

    bool encode(std::ostream& os, const mfast::sequence_cref& seq)
    {
      encode_detail::json_visitor visitor(os);
      visitor.visit(seq, 0);
      return os.good();
    }

  } // namespace json
} // namespace mfast
