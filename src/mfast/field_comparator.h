#ifndef FIELD_COMPARATOR_H_NPPC6W1A
#define FIELD_COMPARATOR_H_NPPC6W1A
#include <utility>
#include "mfast/field_visitor.h"
namespace mfast
{

  namespace detail {
    struct field_uneqaul_exception {};

    class field_comparator
    {
    private:
      field_cref rhs_;

    public:
      enum {
        visit_absent = 1
      };
      field_comparator(const field_cref& rhs)
        : rhs_(rhs)
      {
      }

      template <typename SimpleType>
      void visit(const SimpleType& lhs) const
      {
        SimpleType rhs =  dynamic_cast_as<SimpleType>(rhs_);
        if (lhs != rhs)
          throw field_uneqaul_exception();
      }

      static void compare(const aggregate_cref lhs, const aggregate_cref rhs)
      {
        if (lhs.num_fields() == rhs.num_fields()) {

          for (std::size_t i = 0; i < lhs.num_fields(); ++i) {
            field_comparator comparator(rhs[i]);
            lhs[i].accept_accessor(comparator);
          }
        }
        else {
          throw field_uneqaul_exception();
        }
      }

      void visit(const group_cref& lhs, int) const
      {
        if (lhs.present() != rhs_.present())
          throw field_uneqaul_exception();
        if (lhs.present() )
          compare(lhs, dynamic_cast_as<group_cref>(rhs_));
      }

      void visit(const nested_message_cref& lhs, int) const
      {
        if (lhs.present() != rhs_.present())
          throw field_uneqaul_exception();
        if (lhs.present() )
          compare(lhs, dynamic_cast_as<nested_message_cref>(rhs_));
      }


      void visit(const sequence_cref& lhs, int) const
      {
        sequence_cref rhs = dynamic_cast_as<sequence_cref>(rhs_);
        if (lhs.num_fields() == rhs.num_fields() && lhs.size() == rhs.size()) {
          for (std::size_t i = 0; i < lhs.size(); ++i) {
            compare(lhs[i], rhs[i]);
          }
        }
        else
          throw field_uneqaul_exception();
      }

    };

  }

  inline bool operator == (const aggregate_cref& lhs, const aggregate_cref& rhs)
  {
    try {
      detail::field_comparator::compare(lhs, rhs);
      return true;
    }
    catch (...) {
      return false;
    }
  }

  inline bool operator == (const field_cref& lhs, const field_cref& rhs)
  {
    try {
      detail::field_comparator comparator(rhs);
      lhs.accept_accessor(comparator);
      return true;
    }
    catch (...) {
      return false;
    }
  }

  using namespace std::rel_ops;

}

#endif /* end of include guard: FIELD_COMPARATOR_H_NPPC6W1A */
