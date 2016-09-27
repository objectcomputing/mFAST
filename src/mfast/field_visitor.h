// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "int_ref.h"
#include "enum_ref.h"
#include "decimal_ref.h"
#include "string_ref.h"
#include "group_ref.h"
#include "sequence_ref.h"
#include "message_ref.h"
#include "nested_message_ref.h"
namespace mfast {
namespace detail {
template <typename Result> struct result_holder {
  Result result_;
  Result get_result() const { return result_; }
  template <typename Visitor, typename SimpleRef>
  void apply_visitor(Visitor &v, SimpleRef ref) {
    result_ = v.visit(ref);
  }

  template <typename Visitor, typename CompositeRef>
  void apply_visitor(Visitor &v, CompositeRef ref, int) {
    result_ = v.visit(ref, 0);
  }
};

template <> struct result_holder<void> {
  void get_result() const {}
  template <typename Visitor, typename SimpleRef>
  void apply_visitor(Visitor &v, SimpleRef ref) {
    v.visit(ref);
  }

  template <typename Visitor, typename CompositeRef>
  void apply_visitor(Visitor &v, CompositeRef ref, int) {
    v.visit(ref, 0);
  }
};

template <class FieldAccessor,
          class Result = decltype(
              static_cast<FieldAccessor *>(nullptr)->visit(int32_cref()))>
class field_accessor_adaptor : public field_instruction_visitor,
                               private result_holder<Result> {
  FieldAccessor &accssor_;

public:
  field_accessor_adaptor(FieldAccessor &accssor) : accssor_(accssor) {}
  virtual void visit(const int32_field_instruction *inst,
                     void *storage) override {
    int32_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const uint32_field_instruction *inst,
                     void *storage) override {
    uint32_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const int64_field_instruction *inst,
                     void *storage) override {
    int64_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const uint64_field_instruction *inst,
                     void *storage) override {
    uint64_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const decimal_field_instruction *inst,
                     void *storage) override {
    decimal_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const ascii_field_instruction *inst,
                     void *storage) override {
    ascii_string_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const unicode_field_instruction *inst,
                     void *storage) override {
    unicode_string_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const byte_vector_field_instruction *inst,
                     void *storage) override {
    byte_vector_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const int32_vector_field_instruction *inst,
                     void *storage) override {
    int32_vector_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const uint32_vector_field_instruction *inst,
                     void *storage) override {
    uint32_vector_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const int64_vector_field_instruction *inst,
                     void *storage) override {
    int64_vector_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const uint64_vector_field_instruction *inst,
                     void *storage) override {
    uint64_vector_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  virtual void visit(const group_field_instruction *inst,
                     void *storage) override {
    group_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref, 0);
  }

  virtual void visit(const sequence_field_instruction *inst,
                     void *storage) override {
    sequence_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref, 0);
  }

  virtual void visit(const template_instruction *, void *) override {
    // message_cref ref(static_cast<value_storage*>(storage), inst);
    // this->apply_visitor(accssor_, ref, 0);
  }

  virtual void visit(const templateref_instruction *inst,
                     void *storage) override {
    nested_message_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref, 0);
  }

  virtual void visit(const enum_field_instruction *inst,
                     void *storage) override {
    enum_cref ref(static_cast<value_storage *>(storage), inst);
    this->apply_visitor(accssor_, ref);
  }

  using result_holder<Result>::get_result;
};

template <class FieldMutator,
          class Result = decltype(
              static_cast<FieldMutator *>(nullptr)->visit(int32_mref()))>
class field_mutator_adaptor : public field_instruction_visitor,
                              private result_holder<Result> {
  allocator *alloc_;
  FieldMutator &mutator_;

public:
  field_mutator_adaptor(FieldMutator &mutator, allocator *alloc)
      : alloc_(alloc), mutator_(mutator) {}

  virtual void visit(const int32_field_instruction *inst,
                     void *storage) override {
    int32_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const uint32_field_instruction *inst,
                     void *storage) override {
    uint32_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const int64_field_instruction *inst,
                     void *storage) override {
    int64_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const uint64_field_instruction *inst,
                     void *storage) override {
    uint64_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const decimal_field_instruction *inst,
                     void *storage) override {
    decimal_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const ascii_field_instruction *inst,
                     void *storage) override {
    ascii_string_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const unicode_field_instruction *inst,
                     void *storage) override {
    unicode_string_mref ref(alloc_, static_cast<value_storage *>(storage),
                            inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const byte_vector_field_instruction *inst,
                     void *storage) override {
    byte_vector_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const int32_vector_field_instruction *inst,
                     void *storage) override {
    int32_vector_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const uint32_vector_field_instruction *inst,
                     void *storage) override {
    uint32_vector_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const int64_vector_field_instruction *inst,
                     void *storage) override {
    int64_vector_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const uint64_vector_field_instruction *inst,
                     void *storage) override {
    uint64_vector_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  virtual void visit(const group_field_instruction *inst,
                     void *storage) override {
    group_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref, 0);
  }

  virtual void visit(const sequence_field_instruction *inst,
                     void *storage) override {
    sequence_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref, 0);
  }

  virtual void visit(const template_instruction *, void *) override {
    // message_mref ref(alloc_, static_cast<value_storage*>(storage), inst);
    // this->apply_visitor(mutator_, ref, 0);
  }

  virtual void visit(const templateref_instruction *inst,
                     void *storage) override {
    nested_message_mref ref(alloc_, static_cast<value_storage *>(storage),
                            inst);
    this->apply_visitor(mutator_, ref, 0);
  }

  virtual void visit(const enum_field_instruction *inst,
                     void *storage) override {
    enum_mref ref(alloc_, static_cast<value_storage *>(storage), inst);
    this->apply_visitor(mutator_, ref);
  }

  using result_holder<Result>::get_result;
};

} // namespace detail

///////////////////////////////////////////////////////////

template <typename FieldAccessor>
void field_cref::accept_accessor(FieldAccessor &accessor) const {
  detail::field_accessor_adaptor<FieldAccessor> adaptor(accessor);
  this->instruction()->accept(adaptor,
                              const_cast<value_storage *>(this->storage()));
}

template <typename FieldMutator>
inline void field_mref::accept_mutator(FieldMutator &mutator) const {
  detail::field_mutator_adaptor<FieldMutator> adaptor(mutator, this->alloc_);
  this->instruction()->accept(adaptor, this->storage());
}

template <typename T>
inline decltype(static_cast<T *>(nullptr)->visit(int32_cref()))
apply_accessor(T &accessor, field_cref v) {
  detail::field_accessor_adaptor<T> adaptor(accessor);
  v.instruction()->accept(adaptor, const_cast<mfast::value_storage *>(
                                       field_cref_core_access::storage_of(v)));
  return adaptor.get_result();
}

template <typename T>
inline decltype(static_cast<T *>(nullptr)->visit(int32_mref()))
apply_mutator(T &mutator, field_mref v) {
  detail::field_mutator_adaptor<T> adaptor(mutator, v.allocator());
  v.instruction()->accept(adaptor, field_mref_core_access::storage_of(v));
  return adaptor.get_result();
}

// template <typename FieldAccessor>
// inline decltype (static_cast<FieldAccessor*>(nullptr)->visit(int32_cref()) )
// apply_accessor( FieldAccessor& accessor, aggregate_cref v)
// {
//   return accessor.visit(v, 0);
// }
}
