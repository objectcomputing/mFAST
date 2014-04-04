
#include "json.h"

namespace mfast {

  namespace json {


    struct tag_undecoded_input;

    struct undecoded_input
      : public boost::error_info<tag_undecoded_input, std::string>
    {
      undecoded_input(std::istream& is)
        : boost::error_info<tag_undecoded_input, std::string>("")
      {
        std::streambuf* buf = is.rdbuf();
        char unconsumed[127];
        std::streamsize n = buf->sgetn( unconsumed, 127 );
        this->value().assign(unconsumed, n);
      }

    };


    bool get_quoted_string(std::istream&                  strm,
                           std::string*                   pstr,
                           const mfast::byte_vector_mref* pref,
                           bool                           first_quote_extracted=false)
    {
      std::streambuf* buf = strm.rdbuf();

      char c;
      if (!first_quote_extracted) {
        // make sure the first non-whitespace character is a quote

        strm >> std::skipws >> c;

        if (c != '"') {
          strm.setstate(std::ios::failbit);
        }

        if (!strm)
          BOOST_THROW_EXCEPTION(std::runtime_error("No opening quotation mark for a string"));

        if (pref)
          pref->push_back(c);
      }

      if (pstr)
        pstr->clear();

      bool escaped = false;

      do {
        c = buf->sbumpc();

        if (pref)
          pref->push_back(c);

        if (c != EOF) {
          if (!escaped) {
            if (c=='\\') {
              escaped = true;
              continue;
            }
            else if (c == '"') {
              return true;
            }
          }
          else {
            // escape mode
            const char* escapables="\b\f\n\r\t\v'\"\\";
            const char* orig="bfnrtv'\"\\";
            const char* pch = strchr(orig, c);
            if (pch != 0) {
              c = escapables[pch-orig];
              escaped = false;
            }
            else {
              strm.setstate(std::ios::failbit);
              BOOST_THROW_EXCEPTION(std::runtime_error("Not a valid escape character"));
            }
          }
          if (pstr)
            (*pstr) += static_cast<char>(c);
        }
        else {
          strm.setstate(std::ios::failbit);
          BOOST_THROW_EXCEPTION(std::runtime_error("No closing quotation mark for a string"));
        }
      }
      while (1);
      strm.setstate(std::ios::failbit);
      return false;
    }

    bool skip_matching(std::istream&                  strm,
                       char                           left_bracket,
                       const mfast::byte_vector_mref* pref)
    {
      assert(left_bracket == '{' || left_bracket == '[');

      char right_bracket;

      if (left_bracket == '{')
        right_bracket = '}';
      else // if (left_bracket == '[')
        right_bracket = ']';

      std::streambuf* buf = strm.rdbuf();
      int c;
      std::size_t count=1;

      while ( (c = buf->sbumpc()) != EOF )
      {
        if (pref)
          pref->push_back(c);
        if (c == left_bracket)
          ++count;
        else if (c == right_bracket) {
          if ( --count == 0 )
            return true;
        }
        else if (c == '"') {
          if (!get_quoted_string(strm, 0, pref, true))
            return false;
        }
      }
      BOOST_THROW_EXCEPTION(std::runtime_error("No closing bracket found"));
    }

    bool skip_value (std::istream& strm)
    {
      char c1;
      char rest[5];

      strm >> std::skipws >> c1;
      if (c1 == '{' || c1 == '[')
        return skip_matching(strm, c1, 0);
      else if (c1 == '"')
        return get_quoted_string(strm, 0, 0, true);
      else if (c1 == 'n') {
        // check if it's null
        strm.get(rest, 4);
        if (strcmp(rest, "ull")==0)
          return true;
      }
      else if (c1=='t') {
        // check if it's "true"
        strm.get(rest, 4);
        if (strcmp(rest, "rue")==0)
          return true;
      }
      else if (c1 == 'f') {
        // check if it's "false"
        strm.get(rest, 5);
        if (strcmp(rest, "alse")==0)
          return true;
      }
      else if (c1 == '-' || isdigit(c1)) {
        // skip number
        strm.putback(c1);
        double d;
        strm >> d;
        return strm.good();
      }
      strm.setstate(std::ios::failbit);
      BOOST_THROW_EXCEPTION(std::runtime_error("Unknown value"));
      return false;

    }

    bool get_decimal_string(std::istream& strm, std::string& str)
    {
      const int BUFFER_LEN=128;
      char buf[BUFFER_LEN];
      char* ptr = buf;
      str.resize(0);
      char c;

      strm >> std::skipws >> c;
      if (c != '-' && !isdigit(c)) {
        strm.setstate(std::ios::failbit);
        BOOST_THROW_EXCEPTION(std::runtime_error("expect decimal"));
      }
      *ptr++ = c;
      std::streambuf* sbuf = strm.rdbuf();

      bool has_dot = false;
      bool has_exp = false;
      while ( (c = sbuf->sbumpc()) != EOF ) {
        if (isdigit(c)) {}
        else if (!has_dot && !has_exp && c == '.') {
          has_dot = true;
        }
        else if (!has_exp && (c=='e' || c== 'E')) {
          has_exp = true;
          *ptr++ = c;

          c = sbuf->sbumpc();
          if (c != '+' && c != '-' && !isdigit(c))
            break;
        }
        else {
          break;
        }
        *ptr++ = c;
        if (ptr >= buf+BUFFER_LEN-1) {
          str.append(buf, ptr);
          ptr = buf;
        }
      }

      strm.putback(c);
      if (buf != ptr)
        str.append(buf, ptr);
      return true;
    }

    bool parse_array_preamble(std::istream& strm)
    {
      char c;
      strm >> std::skipws >> c;
      if (!strm.good() || c != '[') {
        strm.setstate(std::ios::failbit);
        BOOST_THROW_EXCEPTION(std::runtime_error("expect [ for an array"));
      }

      strm >> std::ws;
      if (strm.peek() == ']') {
        strm >> c;
        return false;
      }
      return true;
    }

    struct decode_visitor
    {
      std::istream& strm_;
      unsigned json_object_tag_mask_;

      enum {
        visit_absent = true
      };

      decode_visitor(std::istream& strm,
                     unsigned      json_object_tag_mask)
        : strm_(strm)
        , json_object_tag_mask_(json_object_tag_mask)
      {
      }

      template <typename NumericTypeMref>
      void visit(const NumericTypeMref& ref)
      {
        typedef typename NumericTypeMref::value_type value_type;
        value_type value;
        if (strm_ >> value, strm_.good())
          ref.as(value);
        else
          BOOST_THROW_EXCEPTION(std::runtime_error("expect number value"));
      }

      void visit(const enum_mref& ref)
      {
        if (ref.is_boolean()) {
          char c1;
          char rest[5];

          strm_ >> std::skipws >> c1;

          switch (c1)
          {
          case '0':
            ref.as(false);
            return;
          case '1':
            ref.as(true);
            return;
          case 'f':
            strm_.get(rest, 5);
            if (strcmp(rest, "alse")==0) {
              ref.as(false);
              return;
            }
            break;
          case 't':
            strm_.get(rest, 4);
            if (strcmp(rest, "rue")==0) {
              ref.as(true);
              return;
            }
            break;
          }

          BOOST_THROW_EXCEPTION(std::runtime_error("expect boolean value"));
        }
        else {
          // treat it is an integer
          this->visit(reinterpret_cast<const uint64_mref&>(ref));
        }

      }

      void visit(const mfast::decimal_mref& ref)
      {
        try {
          // We cannot directly use the following
          //    decimal val; strm_ >> val;
          // That is because boost::multiprecision::number
          // cannot correctly extract value when the number
          // is not terminate with a string separator (i.e. space, tab, etc).
          // For example, the following code would fail
          //    stringstream strm("1.111,");
          //    decimal val; strm_ >> val;
          // However, if you change the code from decimal to double
          // the extraction would succeed.
          std::string str;
          if (get_decimal_string(strm_, str)) {
            ref.as(mfast::decimal(str));
          }
        }
        catch (...) {
          strm_.setstate(std::ios::failbit);
          BOOST_THROW_EXCEPTION(std::runtime_error("expect decimal value"));
        }
      }

      template <typename Char>
      void visit(const mfast::string_mref<Char>& ref)
      {
        std::string str;
        if (get_quoted_string(strm_, &str, 0)) {
          ref.as(str);
        }
      }

      void visit(const mfast::byte_vector_mref& ref)
      {
        if (ref.instruction()->tag().to_uint64() & json_object_tag_mask_) {
          // if the json_object_tag_mask is on, that means the field shouldn't be unpakced
          char c;
          strm_ >> std::skipws >> c;
          if (strm_.good() && c == '{') {
            ref.push_back(c);
            if (skip_matching(strm_, c, &ref)) {
              return;
            }
          }
          strm_.setstate(std::ios::failbit);
          BOOST_THROW_EXCEPTION(std::runtime_error("expect JSON object"));
        }
        else {
          // write it as a hex string
          std::string str;
          if (get_quoted_string(strm_, &str, 0)) {
            ref.resize(str.size()/2+1);
            ptrdiff_t len = byte_vector_field_instruction::hex2binary(str.c_str(), ref.data());
            ref.resize(len);
          }
        }
      }

      template <typename IntType>
      void visit(int_vector_mref<IntType> &ref)
      {
        ref.clear();
        if (!parse_array_preamble(strm_))
          return;

        std::size_t i = 0;
        char c;

        do {
          ref.resize(i + 1);

          strm_ >> ref[i] >> std::skipws >> c;

          if (!strm_.good())
            BOOST_THROW_EXCEPTION(std::runtime_error("expect integer"));
        }
        while (c == ',');

        if (c == ']')
          return;

        strm_.setstate(std::ios::failbit);
        BOOST_THROW_EXCEPTION(std::runtime_error("expect ] or ,"));
      }

      // return false only when the parsed result is empty or error
      bool visit_impl(const mfast::aggregate_mref& ref);

      void visit(const mfast::sequence_mref& ref, int);

      void visit(const mfast::sequence_element_mref& ref, int)
      {
        ref.accept_mutator(*this);
      }

      void visit(const mfast::group_mref& ref, int)
      {
        if (!visit_impl(ref))
          ref.omit();
      }

      void visit(const mfast::nested_message_mref&, int)
      {
        // unsupported;
      }

    };


    // return false only when the parsed result is empty or error
    bool decode_visitor::visit_impl(const mfast::aggregate_mref& ref)
    {

      for (std::size_t i = 0; i < ref.num_fields(); ++i)
      {
        ref[i].omit();
      }

      char c;
      strm_ >> std::skipws >> c;
      if (strm_.good() && c == '{') {
        // strm_ >> std::skipws >> c;

        if (strm_.peek() == '}') {
          strm_ >> c;
          return false;
        }

        do {
          std::string key;
          if (!get_quoted_string(strm_, &key, 0))
            return false;

          strm_ >> c;

          if (!strm_.good() || c != ':')
          {
            strm_.setstate(std::ios::failbit);
            BOOST_THROW_EXCEPTION(std::runtime_error("expect :"));
          }

          int index = ref.field_index_with_name(key.c_str());
          if (index != -1) {
            // we need to check if the value is null
            strm_ >> std::ws;
            if (strm_.good() && strm_.peek() == 'n') {
              char buf[5];
              strm_ >> std::noskipws >> std::setw(5) >> buf;
              if (strncmp(buf, "null", 4) != 0) {
                strm_.setstate(std::ios::failbit);
                BOOST_THROW_EXCEPTION(std::runtime_error("expect null"));
              }
            }
            else
              ref[index].accept_mutator(*this);
          }
          else {
            // the field is unkown to us,
            skip_value(strm_);
          }

          if (!strm_.good() )
            BOOST_THROW_EXCEPTION(std::runtime_error("expect }"));

          strm_ >> c;

        } while (strm_.good() && c == ',');

        if (strm_.good() && c == '}') {
          return true;
        }
      }
      else if (c == 'n') {
        // check if the result is null
        char buf[4];
        strm_ >> std::noskipws >> std::setw(3) >> buf;
        if (strncmp(buf, "ull", 3) == 0)
          return false;
      }
      strm_.setstate(std::ios::failbit);
      BOOST_THROW_EXCEPTION(std::runtime_error("expect {"));
    }

    void decode_visitor::visit(const mfast::sequence_mref& ref, int)
    {
      ref.clear();
      if (!parse_array_preamble(strm_))
        return;

      std::size_t i = 0;
      char c;

      do {
        ref.resize(i + 1);
        sequence_element_mref element = ref[i++];
        if (ref.element_unnamed())
          element.accept_mutator(*this);
        else
          this->visit_impl(element);

        strm_ >> std::skipws >> c;

        if (!strm_.good())
          return;
      }
      while (c == ',');

      if (c == ']')
        return;
      strm_.setstate(std::ios::failbit);
      BOOST_THROW_EXCEPTION(std::runtime_error("expect ]"));
    }

    void decode(std::istream&                is,
                const mfast::aggregate_mref& msg,
                unsigned                     json_object_tag_mask)
    {
      try {
        decode_visitor visitor(is, json_object_tag_mask);
        visitor.visit_impl(msg);
      }
      catch (boost::exception& ex)
      {
        ex << undecoded_input(is);
        throw;
      }
    }

    void decode(std::istream&               is,
                const mfast::sequence_mref& seq,
                unsigned                    json_object_tag_mask)
    {
      try {
        decode_visitor visitor(is, json_object_tag_mask);
        visitor.visit(seq, 0);
      }
      catch (boost::exception& ex)
      {
        ex << undecoded_input(is);
        throw;
      }
    }

  }
}
