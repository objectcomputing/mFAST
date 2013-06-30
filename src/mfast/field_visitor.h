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
#include "mfast/dynamic_ref.h"
#include "mfast/field_storage_helper.h"

namespace mfast {

template <class FieldAccessor>
class field_accessor_adaptor
  : public field_instruction_visitor
{
  FieldAccessor& accssor_;

  value_storage_t& storage_of(const field_cref& ref)
  {
    return detail::field_storage_helper::storage_of(ref);
  }

  public:

    field_accessor_adaptor(FieldAccessor& accssor)
      : accssor_(accssor)
    {
    }

    void visit(const group_cref& ref)
    {
      if (accssor_.pre_visit(ref)) {
        for (int i = 0; i < ref.fields_count(); ++i) {
          field_cref r(ref.const_field(i));
          if (r.present()) {
            r.instruction()->accept(*this, &storage_of(r));
          }
        }
        accssor_.post_visit(ref);
      }
    }

    void visit(const sequence_cref& ref)
    {
      if (accssor_.pre_visit(ref)) {
        for (int j = 0; j < ref.size(); ++j) {
          sequence_element_cref element(ref[j]);
          if (accssor_.pre_visit(j, element)) {
            for (int i = 0; i < ref.fields_count(); ++i) {
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

    void visit(const message_cref& ref)
    {
      if (accssor_.pre_visit(ref)) {
        for (int i = 0; i < ref.fields_count(); ++i) {
          field_cref r(ref.const_field(i));
          if (r.present()) {
            r.instruction()->accept(*this, &storage_of(r));
          }
        }
        accssor_.post_visit(ref);
      }
    }

    virtual void visit(const int32_field_instruction* inst, void* storage)
    {
      int32_cref ref(static_cast<value_storage_t*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const uint32_field_instruction* inst, void* storage)
    {
      uint32_cref ref(static_cast<value_storage_t*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const int64_field_instruction* inst, void* storage)
    {
      int64_cref ref(static_cast<value_storage_t*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const uint64_field_instruction* inst, void* storage)
    {
      uint64_cref ref(static_cast<value_storage_t*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const decimal_field_instruction* inst, void* storage)
    {
      decimal_cref ref(static_cast<value_storage_t*>(storage),inst);
      accssor_.visit(ref);
    }

    virtual void visit(const ascii_field_instruction* inst, void* storage)
    {
      ascii_string_cref ref(static_cast<value_storage_t*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const unicode_field_instruction* inst, void* storage)
    {
      unicode_string_cref ref(static_cast<value_storage_t*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const byte_vector_field_instruction* inst, void* storage)
    {
      byte_vector_cref ref(static_cast<value_storage_t*>(storage), inst);
      accssor_.visit(ref);
    }

    virtual void visit(const group_field_instruction* inst, void* storage)
    {
      group_cref ref(static_cast<value_storage_t*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const sequence_field_instruction* inst, void* storage)
    {
      sequence_cref ref(static_cast<value_storage_t*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const template_instruction* inst, void* storage)
    {
      message_cref ref(static_cast<value_storage_t*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const templateref_instruction* inst, void* s)
    {
      value_storage_t* storage = static_cast<value_storage_t*>(storage);
      message_cref ref(storage, storage->templateref_storage.instruction_storage.instruction_);
      this->visit(ref);
    }

};




class field_mutator_adaptor_base
  : public field_instruction_visitor
{
  protected:
    value_storage_t* field_storage(const group_mref& ref, std::size_t i)
    {
      return ref.field_storage(i);
    }

    value_storage_t* field_storage(const message_mref& ref, std::size_t i)
    {
      return ref.field_storage(i);
    }

    value_storage_t* field_storage(const field_mref& ref)
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
    typedef typename FieldMutator::struct_context context_t;

    field_mutator_adaptor(FieldMutator& mutator)
      : alloc_(0)
      , mutator_(mutator)
    {
    }

    void visit(const group_mref& ref)
    {
      context_t context;
      if (mutator_.pre_visit(ref, context)) {
        for (int i = 0; i < ref.fields_count(); ++i) {
          ref.subinstruction(i)->accept(*this, field_storage(ref, i));
        }
        mutator_.post_visit(ref,context);
      }
    }

    void visit(const sequence_mref& ref)
    {
      if (mutator_.pre_visit(ref)) {
        for (int j = 0; j < ref.size(); ++j) {
          context_t context;
          sequence_element_mref element(ref[j]);
          if (mutator_.pre_visit(j, element, context)) {
            for (int i = 0; i < ref.fields_count(); ++i) {
              field_mref r(element.mutable_field(i));
              r.instruction()->accept(*this, field_storage(r));
            }
            mutator_.post_visit(j, element, context);
          }
        }
        mutator_.post_visit(ref);
      }
    }

    void visit(const message_mref& ref)
    {
      alloc_ = field_allocator(ref);
      context_t context;
      if (mutator_.pre_visit(ref, context)) {
        for (int i = 0; i < ref.fields_count(); ++i) {
          ref.subinstruction(i)->accept(*this, field_storage(ref, i));
        }
        mutator_.post_visit(ref,context);
      }
    }

    virtual void visit(const int32_field_instruction* inst, void* storage)
    {
      int32_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const uint32_field_instruction* inst, void* storage)
    {
      uint32_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const int64_field_instruction* inst, void* storage)
    {
      int64_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const uint64_field_instruction* inst, void* storage)
    {
      uint64_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const decimal_field_instruction* inst, void* storage)
    {
      decimal_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const ascii_field_instruction* inst, void* storage)
    {
      ascii_string_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const unicode_field_instruction* inst, void* storage)
    {
      unicode_string_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const byte_vector_field_instruction* inst, void* storage)
    {
      byte_vector_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      mutator_.visit(ref);
    }

    virtual void visit(const group_field_instruction* inst, void* storage)
    {
      group_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const sequence_field_instruction* inst, void* storage)
    {
      sequence_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const template_instruction* inst, void* storage)
    {
      message_mref ref(alloc_, static_cast<value_storage_t*>(storage), inst);
      this->visit(ref);
    }

    virtual void visit(const templateref_instruction* inst, void* storage)
    {
      value_storage_t* v = static_cast<value_storage_t*>(storage);
      context_t context;
      dynamic_mref dyn_mref(alloc_, v, inst);
      if (mutator_.pre_visit(dyn_mref, context)) {
        message_mref mref(alloc_, v, v->templateref_storage.instruction_storage.instruction_);
        for (int i = 0; i < mref.fields_count(); ++i) {
          mref.subinstruction(i)->accept(*this, field_storage(mref, i));
        }
        mutator_.post_visit(dyn_mref,context);
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
  field_mutator_adaptor<FieldMutator> adaptor(mutator);
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
  field_mutator_adaptor<FieldMutator> adaptor(mutator);
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
  field_mutator_adaptor<FieldMutator> adaptor(mutator);
  adaptor.visit(*this);
}

}
#endif /* end of include guard: FIELD_VISITOR_H_84G4WWEY */
