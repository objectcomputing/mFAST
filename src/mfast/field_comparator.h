#ifndef FIELD_COMPARATOR_H_NPPC6W1A
#define FIELD_COMPARATOR_H_NPPC6W1A
#include <utility>
#include "mfast/field_visitor.h"
namespace mfast
{

  namespace detail {
    struct field_uneqaul_exception {};

    template <typename T>
    class field_comparator
    {
    private:
      const T& parent_;

    public:

      enum {
        visit_absent = 1
      };

      field_comparator(const T& rhs)
        : parent_(rhs)
      {
      }

      void visit(const group_cref& lhs, int) const
      {
        group_cref rhs =  dynamic_cast_as<group_cref>(parent_[lhs.instruction()->field_index()]);

        if (lhs.num_fields() != rhs.num_fields())
          throw field_uneqaul_exception ();

        field_comparator<group_cref> new_comp(rhs);
        lhs.accept_accessor( new_comp );
      }

      void visit(const sequence_cref& lhs, int) const;

      void visit(const sequence_element_cref&, int) const
      {
      }

      void visit(const nested_message_cref& lhs, int) const
      {
        nested_message_cref rhs =  dynamic_cast_as<nested_message_cref>(parent_[lhs.instruction()->field_index()]);
        field_comparator<message_cref> new_comp(rhs.target());
        lhs.target().accept_accessor( new_comp );
      }

      template <typename SimpleType>
      void visit(const SimpleType& lhs) const
      {
        SimpleType rhs =  dynamic_cast_as<SimpleType>(parent_[lhs.instruction()->field_index()]);
        if (lhs != rhs)
          throw field_uneqaul_exception();
      }

    };

    template <>
    class field_comparator<sequence_cref>
    {
    public:

    private:
      const sequence_cref& parent_;

    public:
      enum {
        visit_absent = 1
      };

      field_comparator(const sequence_cref& rhs)
        : parent_(rhs)
      {
      }

      void visit(const sequence_element_cref& lhs, int index) const
      {
        sequence_element_cref rhs =  parent_[index];
        field_comparator<sequence_element_cref> new_comp(rhs);
        lhs.accept_accessor( new_comp );
      }

      template <typename CompositeType>
      void visit(const CompositeType&, int) const
      {
      }

      template <typename SimpleType>
      void visit(const SimpleType&) const
      {
      }

    };

    template <typename T>
    void field_comparator<T>::visit(const sequence_cref& lhs, int) const
    {
      sequence_cref rhs =  dynamic_cast_as<sequence_cref>(parent_[lhs.instruction()->field_index()]);

      if (lhs.num_fields() != rhs.num_fields() || lhs.size() != rhs.size())
        throw field_uneqaul_exception ();

      field_comparator<sequence_cref> new_comp(rhs);
      lhs.accept_accessor( new_comp );
    }

    template <typename Ref>
    inline bool equal(const Ref& lhs, const Ref& rhs)
    {
      try {
        field_comparator<Ref> comparator(rhs);
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

  inline bool operator == (const sequence_cref& lhs, const sequence_cref& rhs)
  {
    return detail::equal(lhs, rhs);
  }

  inline bool operator == (const aggregate_cref& lhs, const aggregate_cref& rhs)
  {
    return detail::equal(lhs, rhs);
  }

  using namespace std::rel_ops;

}

#endif /* end of include guard: FIELD_COMPARATOR_H_NPPC6W1A */
