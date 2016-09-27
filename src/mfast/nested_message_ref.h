// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_instructions.h"
#include "field_ref.h"
#include "message_ref.h"
#include "type_category.h"

namespace mfast {
class nested_message_cref : public field_cref {
public:
  typedef templateref_instruction instruction_type;
  typedef const templateref_instruction *instruction_cptr;
  static const bool is_mutable = false;
  typedef group_type_tag type_category;

  nested_message_cref() {}
  nested_message_cref(const value_storage *storage, instruction_cptr inst)
      : field_cref(storage, inst) {}

  explicit nested_message_cref(const field_cref &other) : field_cref(other) {}
  const templateref_instruction *instruction() const {
    return static_cast<const templateref_instruction *>(instruction_);
  }

  message_cref target() const {
    return message_cref(this->storage()->of_templateref.content_,
                        this->target_instruction());
  }

  const template_instruction *target_instruction() const {
    return storage()->of_templateref.of_instruction.instruction_;
  }

  operator aggregate_cref() const {
    return aggregate_cref(this->storage()->of_templateref.content_,
                          this->target_instruction());
  }

  template <typename Visitor> void accept(Visitor &v) const { v.visit(*this); }
};

class nested_message_mref : public make_field_mref<nested_message_cref> {
public:
  static const bool is_mutable = true;
  typedef mfast::allocator allocator_type;

  nested_message_mref() {}
  nested_message_mref(allocator_type *alloc, value_storage *storage,
                      const templateref_instruction *inst)
      : make_field_mref<nested_message_cref>(alloc, storage, inst) {}

  nested_message_mref(const field_mref_base &other)
      : make_field_mref<nested_message_cref>(other) {}

  message_mref target() const {
    return message_mref(alloc_, this->storage()->of_templateref.content_,
                        this->target_instruction());
  }

  operator aggregate_mref() const {
    return aggregate_mref(alloc_, storage()->of_templateref.content_,
                          this->target_instruction());
  }

  template <typename Visitor> void accept(Visitor &v) const { v.visit(*this); }

  template <typename MESSAGE> typename MESSAGE::mref_type as() const {
    typedef typename MESSAGE::mref_type mref_type;
    set_target_instruction(MESSAGE::instruction(), true);
    return mref_type(alloc_, this->storage()->of_templateref.content_,
                     MESSAGE::instruction());
  }

  message_mref rebind(const template_instruction *inst) const {
    set_target_instruction(inst, true);
    return message_mref(alloc_, this->storage()->of_templateref.content_, inst);
  }

  void set_target_instruction(const template_instruction *inst,
                              bool construct_subfields = true) const {
    const templateref_instruction *templateRef_inst =
        static_cast<const templateref_instruction *>(this->instruction_);
    const template_instruction *&target_inst =
        this->storage()->of_templateref.of_instruction.instruction_;

    // assert(target_inst == 0);

    if (inst == target_inst)
      return;

    if (target_inst) {
      templateRef_inst->destruct_value(*this->storage(), alloc_);
    }
    templateRef_inst->construct_value(*this->storage(), alloc_, inst,
                                      construct_subfields);
  }
};
}
