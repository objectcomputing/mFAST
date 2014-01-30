
#include "json.h"

namespace mfast {

  namespace json {

    bool get_quoted_string(std::istream& strm, std::string* pstr, bool first_quote_extracted=false)
    {
      char c1;
      if (!first_quote_extracted) {
        // make sure the first non-whitespace character is a quote
        strm >> std::skipws >> c1;
        if (c1 != '"') {
          strm.setstate(std::ios::failbit);
        }
        if (!strm)
          return false;
      }

      if (pstr)
        pstr->clear();

      std::streambuf* buf = strm.rdbuf();
      int c;
      bool escaped = false;

      do {
        c = buf->sbumpc();
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
              break;
            }
          }
          if (pstr)
            (*pstr) += static_cast<char>(c);
        }
        else {
          break;
        }
      }
      while (1);
      strm.setstate(std::ios::failbit);
      return false;
    }

    bool skip_matching(std::istream& strm, char left_bracket)
    {
      char right_bracket;

      if (left_bracket == '{')
        right_bracket = '}';
      else if (left_bracket == '[')
        right_bracket = ']';
      else {
        strm.setstate(std::ios::failbit);
        return false;
      }

      std::streambuf* buf = strm.rdbuf();
      int c;
      std::size_t count=1;

      while ( (c = buf->sbumpc()) != EOF )
      {
        if (c == left_bracket)
          ++count;
        else if (c == right_bracket) {
          if ( --count == 0 )
            return true;
        }
        else if (c == '"') {
          if (!get_quoted_string(strm, 0, true))
            return false;
        }
      }
      return false;
    }

    bool skip_value (std::istream& strm)
    {
      char c1;
      char rest[5];

      strm >> std::skipws >> c1;
      if (c1 == '{' || c1 == '[')
        return skip_matching(strm, c1);
      else if (c1 == '"')
        return get_quoted_string(strm, 0, true);
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
        return false;
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
      if (!strm.good())
        return false;

      if (c != '[') {
        strm.setstate(std::ios::failbit);
        return false;
      }

      strm >> std::skipws;
      if (strm.peek() == ']') {
        strm >> c;
        return false;
      }
      return true;
    }

    struct decode_visitor
    {
      std::istream& strm_;

      enum {
        visit_absent = true
      };

      decode_visitor(std::istream& strm)
        : strm_(strm)
      {
      }

      template <typename NumericTypeMref>
      void visit(const NumericTypeMref& ref)
      {
        typedef typename NumericTypeMref::value_type value_type;
        value_type value;
        if (strm_ >> value, strm_.good())
          ref.as(value);
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

          strm_.setstate(std::ios::failbit);
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
        }
      }

      template <typename Char>
      void visit(const mfast::string_mref<Char>& ref)
      {
        std::string str;
        if (get_quoted_string(strm_, &str)) {
          ref.as(str);
        }
      }

      void visit(const mfast::byte_vector_mref&)
      {
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
            return;
        }
        while (c == ',');

        if (c == ']')
          return;

        strm_.setstate(std::ios::failbit);
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

        if (!strm_.good())
          return false;

        if (strm_.peek() == '}') {
          strm_ >> c;
          return false;
        }

        do {
          std::string key;
          if (!get_quoted_string(strm_, &key))
            return false;

          strm_ >> c;

          if (!strm_.good() || c != ':')
          {
            strm_.setstate(std::ios::failbit);
            return false;
          }

          int index = ref.field_index_with_name(key.c_str());
          if (index != -1)
            ref[index].accept_mutator(*this);
          else {
            // the field is unkown to us,
            skip_value(strm_);
          }

          if (!strm_.good() )
            return false;

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
      return false;
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
    }

    bool decode(std::istream& is, const mfast::aggregate_mref& msg)
    {
      decode_visitor visitor(is);
      visitor.visit_impl(msg);
      return !is.fail();
    }

    bool decode(std::istream& is, const mfast::sequence_mref& seq)
    {
      decode_visitor visitor(is);
      visitor.visit(seq, 0);
      return !is.fail();
    }

  }
}
