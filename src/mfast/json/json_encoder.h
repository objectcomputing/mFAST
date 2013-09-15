#ifndef JSON_ENCODER_H_DHG4BF3O
#define JSON_ENCODER_H_DHG4BF3O

#include <mfast.h>
#include <iostream>

namespace mfast {
namespace detail {

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

bool is_empty(const mfast::aggregate_cref& ref)
{
  for (std::size_t i = 0; i < ref.num_fields(); ++i)
  {
    if (ref[i].present())
      return false;
  }
  return true;
}

}


class json_value_visitor
{
  private:
    std::ostream& strm_;
    char separator_[2];

  public:

    enum {
      visit_absent = 0
    };

    json_value_visitor(std::ostream& strm)
      : strm_(strm)
    {
      separator_[0] = 0;
      separator_[1] = 0;
    }

    template <typename NumericTypeRef>
    void visit(const NumericTypeRef& ref)
    {
      strm_ <<  separator_ << ref.value();
      separator_[0] = ',';
    }

    void visit(const mfast::ascii_string_cref& ref)
    {
      strm_ <<  separator_ << detail::quoted_string(ref.c_str());
      separator_[0] = ',';
    }

    void visit(const mfast::unicode_string_cref& ref)
    {
      strm_ <<  separator_ << detail::quoted_string(ref.c_str());
      separator_[0] = ',';
    }

    void visit(const mfast::byte_vector_cref& ref)
    { // json doesn't have byte vector, treat it as string now
      strm_ <<  separator_ << detail::quoted_string(reinterpret_cast<const char*>(ref.data()));
      separator_[0] = ',';
    }

    void visit(const mfast::group_cref&, int)
    {
    }

    void visit(const mfast::sequence_cref& ref, int);

    void visit(const mfast::sequence_element_cref& ref, int)
    {
      ref.accept_accessor(*this);
    }

    void visit(const mfast::nested_message_cref& ref, int);
};


class json_object_visitor
{
  private:
    std::ostream& strm_;
    char separator_[2];

  public:

    enum {
      visit_absent = 0
    };

    json_object_visitor(std::ostream& strm)
      : strm_(strm)
    {
      separator_[0] = 0;
      separator_[1] = 0;
    }

    template <typename NumericTypeRef>
    void visit(const NumericTypeRef& ref)
    {
      strm_ << separator_ << detail::quoted_string(ref.name()) << ":" << ref.value();
      separator_[0] = ',';
    }

    void visit(const mfast::ascii_string_cref& ref)
    {
      strm_ << separator_ << detail::quoted_string(ref.name()) << ":" << detail::quoted_string(ref.c_str());
      separator_[0] = ',';
    }

    void visit(const mfast::unicode_string_cref& ref)
    {
      strm_ << separator_<< detail::quoted_string(ref.name()) << ":" << detail::quoted_string(ref.c_str());
      separator_[0] = ',';
    }

    void visit(const mfast::byte_vector_cref& ref)
    { // json doesn't have byte vector, treat it as string now
      strm_ << separator_<< detail::quoted_string(ref.name()) << ":" << detail::quoted_string(reinterpret_cast<const char*>(ref.data()));
      separator_[0] = ',';
    }

    void visit(const mfast::group_cref& ref, int)
    {
      if (detail::is_empty(ref))
        return;
      strm_ << separator_ << detail::quoted_string(ref.name()) << ":{";

      separator_[0] = '\0';
      ref.accept_accessor(*this);

      strm_ << "}";
      separator_[0] = ',';
    }
    
    void visit(const mfast::nested_message_cref&  ref, int)
    {
      if (detail::is_empty(ref))
        return;

      ref.accept_accessor(*this);
      separator_[0] = ',';
    }

    void visit(const mfast::sequence_cref& ref, int)
    {
      strm_ << separator_ << detail::quoted_string(ref.name()) << ":[";
      if (ref.num_fields() > 1) {
        separator_[0] = '\0';
        ref.accept_accessor(*this);
      }
      else {
        json_value_visitor sub_visitor(strm_);
        ref.accept_accessor(sub_visitor);
      }
      strm_ << "]";
      separator_[0] = ',';
    }

    void visit(const mfast::sequence_element_cref& ref, int);
};



inline bool json_encode(std::ostream& os, const mfast::aggregate_cref& msg)
{
  os << "{";
  json_object_visitor visitor(os);
  msg.accept_accessor(visitor);
  os << "}";
  return os.good();
}

inline 
void json_value_visitor::visit(const mfast::sequence_cref& ref, int)
{

  // should only be called for sequence of sequence
  strm_ << separator_ <<  "[";
  if (ref.num_fields() > 1) {
    json_object_visitor sub_visitor(strm_);
    ref.accept_accessor(sub_visitor);
  }
  else {
    separator_[0] = '\0';
    ref.accept_accessor(*this);
  }
  strm_ << "]";
  separator_[0] = ',';
}

void json_value_visitor::visit(const mfast::nested_message_cref& ref, int)
{
  json_encode(strm_, ref);
}

inline void 
json_object_visitor::visit(const mfast::sequence_element_cref& ref, int)
{
  strm_ << separator_ ;
  json_encode(strm_, ref);
  separator_[0] = ',';
}
}
#endif /* end of include guard: JSON_ENCODER_H_DHG4BF3O */
