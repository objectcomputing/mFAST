#include "json.h"
#include <boost/io/ios_state.hpp>
#include <boost/type_traits.hpp>
#include <cstdio>

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

      // Although C99 defined PRId64 macro in <inttypes.h> as the printf flag for int64_t,
      // it's not available on MSVC. On the travis-ci platform, <inttypes.h> is available,
      // but PRId64 is undefined. Using function overload is the only way I can make it
      // portable without compiler warnining.
      inline int snprint_int(char* buf, int buf_size, long x)
      {
#ifndef _MSC_VER
        return std::snprintf(buf, buf_size, "%ld", x);
#else
        return sprintf_s(buf, buf_size, "%ld", x);
#endif
      }

      inline int snprint_int(char* buf, int buf_size, long long x)
      {
#ifndef _MSC_VER
        return std::snprintf(buf, buf_size, "%lld", x);
#else
        return sprintf_s(buf, buf_size, "%lld", x);
#endif
	  }


      std::ostream& operator << (std::ostream& os, const decimal_value_storage& storage)
      {
        int64_t mantissa =  storage.mantissa();
        int exponent = storage.exponent();

        if (exponent >= 0) {
          os << mantissa;
          std::fill_n(std::ostream_iterator<char>(os), exponent, '0');
        }
        else if (exponent < 0) {
          char buf[128];

          int n= snprint_int(buf, 128, mantissa);

          char* p = buf;
          if (mantissa < 0) {
            os.put('-');
            --n;
            ++p;
          }
          if ((n+exponent) > 0) {
            os.write(p, n+exponent);
            os << '.' << p+n+exponent;
          }
          else {
            os << "0.";
            std::fill_n(std::ostream_iterator<char>(os), -exponent-n , '0');
            os << p;
          }
        }

        return os;
      }

      class json_visitor
      {
      private:
        std::ostream& strm_;
        char separator_[2];
        unsigned json_object_tag_mask_;

      public:

        enum {
          visit_absent = 0
        };

        json_visitor(std::ostream& strm,
                     unsigned      json_object_tag_mask)
          : strm_(strm)
          , json_object_tag_mask_(json_object_tag_mask)
        {
          separator_[0] = 0;
          separator_[1] = 0;
        }

        template <typename NumericTypeRef>
        void visit(const NumericTypeRef& ref)
        {
          strm_ <<  separator_ << ref.value();
        }

        void visit(const decimal_cref& ref)
        {
          const decimal_value_storage& storage = *reinterpret_cast<const decimal_value_storage*>( field_cref_core_access::storage_of(ref) );
          strm_ <<  separator_ << storage;
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
        {
          if (ref.instruction()->tag().to_uint64() & json_object_tag_mask_)
          {
            // if the json_object_tag_mask is on, that means the field contains
            // json encoded object already, just write it as it is without any processing.

            strm_.rdbuf()->sputn( reinterpret_cast<const char*>(ref.data()), ref.size());
          }
          else
          {
            // json doesn't have byte vector, treat it as hex string now
            strm_ <<  separator_ << "\"";
            boost::io::ios_flags_saver ifs( strm_ );
            strm_ << std::hex << std::setfill('0') << std::setw(2);

            for (std::size_t i = 0; i < ref.size(); ++i)
            {
              // if the size is 16, we treat it as a UUID
              if (ref.size() == 16 && (i==4 || i==6 || i==8 || i==10))
                strm_ << '-';
              strm_ <<  (0xFF & (int) ref[i]);
            }
            strm_ << "\"" << std::setfill(' ');
          }
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
              separator_[0] = ',';
            }
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

    bool encode(std::ostream&                os,
                const mfast::aggregate_cref& msg,
                unsigned                     json_object_tag_mask)
    {
      encode_detail::json_visitor visitor(os, json_object_tag_mask);
      visitor.visit(msg, 0);
      return os.good();
    }

    bool encode(std::ostream&               os,
                const mfast::sequence_cref& seq,
                unsigned                    json_object_tag_mask)
    {
      encode_detail::json_visitor visitor(os, json_object_tag_mask);
      visitor.visit(seq, 0);
      return os.good();
    }

  } // namespace json
} // namespace mfast
