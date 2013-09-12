// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef FIELD_VISITOR_H_84G4WWEY
#define FIELD_VISITOR_H_84G4WWEY

#include "mfast/int_ref.h"
#include "mfast/decimal_ref.h"
#include "mfast/string_ref.h"
#include "mfast/group_ref.h"
#include "mfast/sequence_ref.h"
#include "mfast/message_ref.h"
#include "mfast/nested_message_ref.h"
#include <boost/utility/enable_if.hpp>

namespace mfast {

namespace detail {

template <class FieldAccessor>
class field_accessor_adaptor
  : public field_instruction_visitor
  , private detail::field_storage_helper
{
  FieldAccessor& accssor_;

  public:

    field_accessor_adaptor(FieldAccessor& accssor)
      : accssor_(accssor)
    {
    }

    void visit(const aggregate_cref& ref)
    {
      for (std::size_t i = 0; i < ref.num_fields(); ++i) {
        field_cref r(ref[i]);
        if (r.present() || FieldAccessor::visit_absent ) {
          r.instruction()->accept(*this, storage_ptr_of(r));
        }
      }
    }

    void visit(const sequence_cref& ref)
    {
      for (std::size_t j = 0; j < ref.size(); ++j) {
        sequence_element_cref element(ref[j]);
        accssor_.visit(element, j);
      }
    }

    virtual void visit(const int32_field_instruction* inst, void* storage)
    {
      int32_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const uint32_field_instruction* inst, void* storage)
    {
      uint32_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const int64_field_instruction* inst, void* storage)
    {
      int64_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const uint64_field_instruction* inst, void* storage)
    {
      uint64_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const decimal_field_instruction* inst, void* storage)
    {
      decimal_cref ref(static_cast<value_storage*>(storage),inst);
      accssor_.visit(ref);
    }

    virtual void visit(const ascii_field_instruction* inst, void* storage)
    {
      ascii_string_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const unicode_field_instruction* inst, void* storage)
    {
      unicode_string_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const byte_vector_field_instruction* inst, void* storage)
    {
      byte_vector_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const group_field_instruction* inst, void* storage)
    {
      group_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref, 0);
    }

    virtual void visit(const sequence_field_instruction* inst, void* storage)
    {
      sequence_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref, 0);
    }

    virtual void visit(const template_instruction*, void* )
    {
    }

    virtual void visit(const templateref_instruction* inst, void* storage)
    {
      nested_message_cref ref(static_cast<value_storage*>(storage), inst);
      accssor_.visit(ref, 0);
    }

};


template <class FieldMutator>
class field_mutator_adaptor
  : public field_instruction_visitor
  , private detail::field_storage_helper
{
  allocator* alloc_;
  FieldMutator& mutator_;

  public:

    field_mutator_adaptor(FieldMutator& mutator, allocator* alloc)
      : alloc_(alloc)
      , mutator_(mutator)
    {
    }

    void visit(const aggregate_mref& ref)
    {
      for (std::size_t i = 0; i < ref.num_fields(); ++i) {
        field_mref r(ref[i]);
        if (r.present() || FieldMutator::visit_absent ) {
          r.instruction()->accept(*this, storage_ptr_of(r));
        }
      }
    }

    void visit(const sequence_mref& ref)
    {
      for (std::size_t j = 0; j < ref.size(); ++j) {
        sequence_element_mref element(ref[j]);
        mutator_.visit(element, j);
      }
    }

    virtual void visit(const int32_field_instruction* inst, void* storage)
    {
      int32_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const uint32_field_instruction* inst, void* storage)
    {
      uint32_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const int64_field_instruction* inst, void* storage)
    {
      int64_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const uint64_field_instruction* inst, void* storage)
    {
      uint64_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const decimal_field_instruction* inst, void* storage)
    {
      decimal_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const ascii_field_instruction* inst, void* storage)
    {
      ascii_string_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const unicode_field_instruction* inst, void* storage)
    {
      unicode_string_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const byte_vector_field_instruction* inst, void* storage)
    {
      byte_vector_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const group_field_instruction* inst, void* storage)
    {
      group_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref, 0);
    }

    virtual void visit(const sequence_field_instruction* inst, void* storage)
    {
      sequence_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref, 0);
    }

    virtual void visit(const template_instruction*, void* )
    {
    }

    virtual void visit(const templateref_instruction* inst, void* storage)
    {
      nested_message_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
      mutator_.visit(ref, 0);
    }

};

} // namespace detail

///////////////////////////////////////////////////////////


template <typename FieldAccessor>
inline void
aggregate_cref::accept_accessor(FieldAccessor& accessor) const
{
  detail::field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  adaptor.visit(*this);
}

template <typename ConstRef>
template <typename FieldMutator>
inline void
make_aggregate_mref<ConstRef>::accept_mutator(FieldMutator& mutator) const
{
  detail::field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  adaptor.visit(*this);
}

template <typename FieldAccessor>
inline void
group_cref::accept_accessor(FieldAccessor& accessor) const
{
  detail::field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  adaptor.visit(*this);
}

template <typename ConstFieldRef>
template <typename FieldMutator>
inline void
make_group_mref<ConstFieldRef>::accept_mutator(FieldMutator& mutator) const
{
  detail::field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  adaptor.visit(*this);
}

template <typename ElementType, typename SequenceInstructionType>
template <typename FieldAccessor>
inline void
make_sequence_cref<ElementType,SequenceInstructionType>::accept_accessor(FieldAccessor& accessor) const
{
  detail::field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  adaptor.visit(*this);
}

template <typename ElementType, typename SequenceInstructionType>
template <typename FieldMutator>
inline void
make_sequence_mref<ElementType,SequenceInstructionType>::accept_mutator(FieldMutator& mutator) const
{
  detail::field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  adaptor.visit(*this);
}

// template <typename FieldAccessor>
// inline void
// message_cref::accept_accessor(FieldAccessor& accessor) const
// {
//   detail::field_accessor_adaptor<FieldAccessor> adaptor(accessor);
//   adaptor.visit(*this);
// }

template <typename FieldAccessor>
inline void nested_message_cref::accept_accessor(FieldAccessor& accessor) const
{
  detail::field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  adaptor.visit(*this);
}

// template <typename ConstFieldRef>
// template <typename FieldMutator>
// inline void
// make_message_mref<ConstFieldRef>::accept_mutator(FieldMutator& mutator) const
// {
//   detail::field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
//   adaptor.visit(*this);
// }

template <typename FieldMutator>
inline void nested_message_mref::accept_mutator(FieldMutator& mutator) const
{
  detail::field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  adaptor.visit(*this);
}

}
#endif /* end of include guard: FIELD_VISITOR_H_84G4WWEY */
