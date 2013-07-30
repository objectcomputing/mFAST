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
#include "mfast/dynamic_message_ref.h"

namespace mfast {


struct field_accessor_base
{
  typedef mfast::group_cref group_ref_type;
  typedef mfast::sequence_cref sequence_ref_type;
  typedef mfast::sequence_element_cref sequence_element_ref_type;
  typedef mfast::message_cref message_ref_type;
  typedef mfast::dynamic_message_cref dynamic_message_ref_type;

};

template <class FieldAccessor>
class field_accessor_adaptor
  : public field_instruction_visitor
{
  FieldAccessor& accssor_;

  value_storage& storage_of(const field_cref& ref)
  {
    return detail::field_storage_helper::storage_of(ref);
  }

  public:

    typedef typename FieldAccessor::group_ref_type group_ref_type;
    typedef typename FieldAccessor::sequence_ref_type sequence_ref_type;
    typedef typename FieldAccessor::sequence_element_ref_type sequence_element_ref_type;
    typedef typename FieldAccessor::message_ref_type message_ref_type;
    typedef typename FieldAccessor::dynamic_message_ref_type dynamic_message_ref_type;

    field_accessor_adaptor(FieldAccessor& accssor)
      : accssor_(accssor)
    {
    }

    template <typename RefType>
    void visit(RefType& ref)
    {
      if (accssor_.pre_visit(ref)) {
        for (std::size_t i = 0; i < ref.fields_count(); ++i) {
          field_cref r(ref.const_field(i));
          if (r.present()) {
            r.instruction()->accept(*this, &storage_of(r));
          }
        }
        accssor_.post_visit(ref);
      }
    }

    void visit(sequence_ref_type& ref)
    {
      if (accssor_.pre_visit(ref)) {
        for (std::size_t j = 0; j < ref.size(); ++j) {
          sequence_element_ref_type element(ref[j]);
          if (accssor_.pre_visit(j, element)) {
            for (std::size_t i = 0; i < ref.fields_count(); ++i) {
              field_cref r(element.const_field(i));
              if (r.present()) {
                r.instruction()->accept(*this, &storage_of(r));
              }
            }
            accssor_.post_visit(j, element);
          }
        }
        accssor_.post_visit(ref);
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
      group_ref_type ref(static_cast<value_storage*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const sequence_field_instruction* inst, void* storage)
    {
      sequence_ref_type ref(static_cast<value_storage*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const template_instruction* inst, void* storage)
    {
      message_ref_type ref(static_cast<value_storage*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const templateref_instruction* inst, void* storage)
    {
      value_storage* v = static_cast<value_storage*>(storage);
      dynamic_message_ref_type dyn_cref(v, inst);
      if (accssor_.pre_visit(dyn_cref)) {
        mfast::message_cref ref(v, dyn_cref.instruction());

        for (std::size_t i = 0; i < ref.fields_count(); ++i) {
          field_cref r(ref.const_field(i));
          if (r.present()) {
            r.instruction()->accept(*this, &storage_of(r));
          }
        }

        accssor_.post_visit(dyn_cref);
      }
    }

};


struct field_mutator_base
{
  typedef mfast::group_mref group_ref_type;
  typedef mfast::sequence_mref sequence_ref_type;
  typedef mfast::sequence_element_mref sequence_element_ref_type;
  typedef mfast::message_mref message_ref_type;
  typedef mfast::dynamic_message_mref dynamic_message_ref_type;
};


class field_mutator_adaptor_base
  : public field_instruction_visitor
{
  protected:
    value_storage* field_storage(const group_mref& ref, std::size_t i)
    {
      return ref.field_storage(i);
    }

    value_storage* field_storage(const message_mref& ref, std::size_t i)
    {
      return ref.field_storage(i);
    }

    value_storage* field_storage(const field_mref& ref)
    {
      return ref.storage();
    }

    allocator* field_allocator(const message_mref& ref)
    {
      return ref.alloc_;
    }

};



template <class FieldMutator>
class field_mutator_adaptor
  : public field_mutator_adaptor_base
  , private detail::field_storage_helper
{
  allocator* alloc_;
  FieldMutator& mutator_;

  public:

    typedef typename FieldMutator::group_ref_type group_ref_type;
    typedef typename FieldMutator::sequence_ref_type sequence_ref_type;
    typedef typename FieldMutator::sequence_element_ref_type sequence_element_ref_type;
    typedef typename FieldMutator::message_ref_type message_ref_type;
    typedef typename FieldMutator::dynamic_message_ref_type dynamic_message_ref_type;

    field_mutator_adaptor(FieldMutator& mutator, allocator* alloc)
      : alloc_(alloc)
      , mutator_(mutator)
    {
    }

    template <typename RefType>
    void visit(RefType& ref)
    {
      if (mutator_.pre_visit(ref)) {
        for (std::size_t i = 0; i < ref.fields_count(); ++i) {
          ref.subinstruction(i)->accept(*this, field_storage(ref, i));
        }
        mutator_.post_visit(ref);
      }
    }

    void visit(sequence_ref_type& ref)
    {
      if (mutator_.pre_visit(ref)) {
        for (std::size_t j = 0; j < ref.size(); ++j) {
          sequence_element_ref_type element(ref[j]);
          if (mutator_.pre_visit(j, element)) {
            for (std::size_t i = 0; i < ref.fields_count(); ++i) {
              field_mref r(element.mutable_field(i));
              r.instruction()->accept(*this, field_storage(r));
            }
            mutator_.post_visit(j, element);
          }
        }
        mutator_.post_visit(ref);
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
      group_ref_type ref(alloc_, static_cast<value_storage*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const sequence_field_instruction* inst, void* storage)
    {
      sequence_ref_type ref(alloc_, static_cast<value_storage*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const template_instruction* inst, void* storage)
    {
      message_ref_type ref(alloc_, static_cast<value_storage*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const templateref_instruction* inst, void* storage)
    {
      value_storage* v = static_cast<value_storage*>(storage);
      dynamic_message_ref_type dyn_mref(alloc_, v, inst);
      if (mutator_.pre_visit(dyn_mref)) {
        message_mref mref(alloc_, v, v->of_templateref.of_instruction.instruction_);
        for (std::size_t i = 0; i < mref.fields_count(); ++i) {
          mref.subinstruction(i)->accept(*this, field_storage(mref, i));
        }
        mutator_.post_visit(dyn_mref);
      }
    }

};

///////////////////////////////////////////////////////////
template <typename FieldAccessor>
inline void
group_cref::accept_accessor(FieldAccessor& accessor) const
{
  field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  adaptor.visit(*this);
}

template <typename ConstFieldRef>
template <typename FieldMutator>
inline void
make_group_mref<ConstFieldRef>::accept_mutator(FieldMutator& mutator) const
{
  field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  adaptor.visit(*reinterpret_cast<group_mref*>(this));
}

template <typename ElementType>
template <typename FieldAccessor>
inline void
make_sequence_cref<ElementType>::accept_accessor(FieldAccessor& accessor) const
{
  field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  adaptor.visit(*reinterpret_cast<sequence_cref*>(this));
}

template <typename ElementType>
template <typename FieldMutator>
inline void
make_sequence_mref<ElementType>::accept_mutator(FieldMutator& mutator) const
{
  field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  adaptor.visit(*reinterpret_cast<sequence_mref*>(this));
}

template <typename FieldAccessor>
inline void
message_cref::accept_accessor(FieldAccessor& accessor) const
{
  field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  adaptor.visit(*this);
}

template <typename ConstFieldRef>
template <typename FieldMutator>
inline void
make_message_mref<ConstFieldRef>::accept_mutator(FieldMutator& mutator) const
{
  field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  adaptor.visit(*this);
}

}
#endif /* end of include guard: FIELD_VISITOR_H_84G4WWEY */
