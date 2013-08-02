#ifndef FIELD_COMPARATOR_H_NPPC6W1A
#define FIELD_COMPARATOR_H_NPPC6W1A

#include "mfast/field_visitor.h"
#include <stack>
namespace mfast
{

namespace detail {
struct field_uneqaul_exception {};

class field_comparator
  : public field_accessor_base
{
  private:
    std::stack<field_cref> others_;
  public:
    
    typedef index_mixin<sequence_element_cref> sequence_element_ref_type;
    
    template <typename Ref>
    field_comparator(const Ref& rhs)
    {
      others_.push(field_cref(rhs));
    }

    template <typename RefType>
    bool pre_visit(const RefType& cref)
    {
      RefType rhs = dynamic_cast_as<RefType>(others_.top());
      if (cref.fields_count() != rhs.fields_count())
        throw field_uneqaul_exception();

      for (int i = cref.fields_count()-1; i>=0; --i) {
        others_.push(rhs.const_field(i));
      }
      return true;
    }

    template <typename RefType>
    void post_visit(const RefType&)
    {
      others_.pop();
    }

    bool pre_visit(const sequence_ref_type& cref)
    {
      sequence_cref rhs = dynamic_cast_as<sequence_cref>(others_.top());
      if (cref.size() != rhs.size())
        throw field_uneqaul_exception();

      return true;
    }
    
    bool pre_visit(const sequence_element_ref_type& cref)
    {
      sequence_cref rhs =  sequence_cref(others_.top());
      sequence_element_cref element = rhs[cref.index];
      for (int i = cref.fields_count()-1; i>=0; --i) {
        others_.push(element.const_field(i));
      }
      return true;
    }
    
    void post_visit(const sequence_element_ref_type&)
    {
    }
    
    
    bool pre_visit(const dynamic_message_cref& cref)
    {
      dynamic_message_cref rhs = dynamic_cast_as<dynamic_message_cref>(others_.top());
      others_.pop();
      others_.push(field_cref(rhs));
      this->pre_visit(message_cref(cref));
      return true;
    }
    
    template <typename PrimitiveRefType>
    void visit(const PrimitiveRefType& ref)
    {
      PrimitiveRefType rhs = dynamic_cast_as<PrimitiveRefType>(others_.top());
      
      if (ref != rhs) {
        throw field_uneqaul_exception();
      }
      others_.pop();
    }
};

template <typename Ref>
inline bool equal(const Ref& lhs, const Ref& rhs) 
{
  try {
    field_comparator comparator(rhs);
    lhs.accept_accessor(comparator);
    return true;
  }
  catch (...) {
    return false;
  }  
}
}


inline bool operator == (const group_cref& lhs, const group_cref& rhs)
{
  return detail::equal(lhs, rhs);
}

inline bool operator != (const group_cref& lhs, const group_cref& rhs)
{
  return !(lhs == rhs);
}

inline bool operator == (const sequence_cref& lhs, const sequence_cref& rhs)
{
  return detail::equal(lhs, rhs);
}

inline bool operator != (const sequence_cref& lhs, const sequence_cref& rhs)
{
  return !(lhs == rhs);
}

inline bool operator == (const message_cref& lhs, const message_cref& rhs)
{
 return detail::equal(lhs, rhs);
}

inline bool operator != (const message_cref& lhs, const message_cref& rhs)
{
  return !(lhs == rhs);
}

}

#endif /* end of include guard: FIELD_COMPARATOR_H_NPPC6W1A */
