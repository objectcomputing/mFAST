// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "exceptions.h"
#include "template_repo.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace mfast {

inline bool is_empty_string(const char *str) {
  return str == nullptr || str[0] == '\0';
}

namespace {
struct tag_key;

typedef boost::error_info<tag_key, std::string> key_info;

struct tag_first_type;

typedef boost::error_info<tag_first_type, std::string> first_type_info;

struct tag_second_type;

typedef boost::error_info<tag_second_type, std::string> second_type_info;

class key_type_mismatch_error : public fast_dynamic_error {
public:
  key_type_mismatch_error(const std::string &keyname,
                          field_type_enum_t first_type,
                          field_type_enum_t second_type)
      : fast_dynamic_error("D4") {
    const char *field_type_name[] = {
        "int32",       "uInt32",      "int64",        "uInt64",
        "decimal",     "exponent",    "ascii string", "unicode string",
        "byteVector",  "int32Vector", "uInt32Vector", "int64Vector",
        "uInt64Vector"};
    *this << key_info(keyname) << first_type_info(field_type_name[first_type])
          << second_type_info(field_type_name[second_type]);
  }
};
}

std::string qualified_name(const char *ns, const char *name) {
  std::string result = ns;
  result += "||";
  result += name;
  return result;
}

dictionary_builder::dictionary_builder(template_repo_base &repo_base)
    : repo_base_(repo_base), alloc_(repo_base.instruction_alloc_) {}

void dictionary_builder::build_group(const field_instruction *fi,
                                     const group_field_instruction *src,
                                     group_field_instruction *dest) {
  const char *inherited_ns = current_ns_;
  const char *inherited_dictionary = current_dictionary_;
  std::string inherited_type = current_type_;

  if (!is_empty_string(fi->ns())) {
    this->current_ns_ = fi->ns();
  }

  if (!is_empty_string(src->dictionary())) {
    this->current_dictionary_ = src->dictionary();
  }

  if (!is_empty_string(src->typeref_name())) {
    const char *type_ns = this->current_ns_;
    if (!is_empty_string(src->typeref_name())) {
      type_ns = src->typeref_ns();
    }
    current_type_ = qualified_name(type_ns, src->typeref_name());
  }

  std::size_t instructions_count = src->subinstructions().size();
  const_instruction_ptr_t *subinstructions =
      static_cast<const_instruction_ptr_t *>(
          alloc_.allocate(instructions_count * sizeof(field_instruction *)));
  for (size_t i = 0; i < instructions_count; ++i) {
    src->subinstruction(i)->accept(*this, &subinstructions[i]);
  }

  dest->set_subinstructions(
      instructions_view_t(subinstructions, instructions_count));

  current_type_ = inherited_type;
  current_ns_ = inherited_ns;
  current_dictionary_ = inherited_dictionary;
}

template_instruction *
dictionary_builder::clone_instruction(const template_instruction *src_inst) {
  uint32_t id = src_inst->id();
  if (repo_base_.get_template(id)) {
    using namespace coder;
    BOOST_THROW_EXCEPTION(duplicate_template_id_error(id)
                          << template_name_info(src_inst->name()));
  }

  auto dest = new (alloc_) template_instruction(*src_inst);

  const char *ns = src_inst->ns();
  if (is_empty_string(ns)) {
    ns = current_ns_;
  }
  current_template_ = qualified_name(ns, src_inst->name());

  this->build_group(src_inst, src_inst, dest);
  return dest;
}

void dictionary_builder::visit(const template_instruction *src_inst,
                               void *dest_inst) {
  template_instruction *&dest =
      *static_cast<template_instruction **>(dest_inst);
  dest = repo_base_.get_template(src_inst->id());
  if (dest == nullptr) {
    BOOST_THROW_EXCEPTION(coder::template_not_found_error(
        src_inst->name(), current_template_.c_str()));
  }
}

void dictionary_builder::visit(const templateref_instruction * /*src_inst*/,
                               void *dest_inst) {
  templateref_instruction *&dest =
      *static_cast<templateref_instruction **>(dest_inst);

  // this is dynamic templateRef, it can only be binded at decoding time
  dest = new (alloc_) templateref_instruction;
}

void dictionary_builder::visit(const group_field_instruction *src_inst,
                               void *dest_inst) {
  group_field_instruction *&dest =
      *static_cast<group_field_instruction **>(dest_inst);

  dest = new (alloc_) group_field_instruction(*src_inst);

  this->build_group(src_inst, src_inst, dest);
}

void dictionary_builder::visit(const sequence_field_instruction *src_inst,
                               void *dest_inst) {
  sequence_field_instruction *&dest =
      *static_cast<sequence_field_instruction **>(dest_inst);
  dest = new (alloc_) sequence_field_instruction(*src_inst);

  this->build_group(src_inst, src_inst, dest);

  if (src_inst->length_instruction()) {
    visit(src_inst->length_instruction(), &dest->sequence_length_instruction_);
  } else {
    dest->sequence_length_instruction_ = new (alloc_)
        uint32_field_instruction(operator_none, presence_mandatory, 0, "", "",
                                 nullptr, int_value_storage<uint32_t>());
  }
}

value_storage *dictionary_builder::get_dictionary_storage(
    const char *key, const char *ns, const op_context_t *op_context,
    field_type_enum_t field_type, value_storage *candidate_storage,
    field_instruction *instruction) {
  operator_enum_t field_operator = instruction->field_operator();

  // except for the specified operators, the field will never depened on
  // previous values
  if (field_operator != operator_delta && field_operator != operator_copy &&
      field_operator != operator_increment && field_operator != operator_tail) {
    return candidate_storage;
  }

  const char *dict = "";

  if (op_context) {
    if (!is_empty_string(op_context->key_))
      key = op_context->key_;
    ns = op_context->ns_;
    dict = op_context->dictionary_;
  }

  if (is_empty_string(dict)) {
    dict = this->current_dictionary_;
    if (is_empty_string(dict)) {
      dict = "global";
    }
  }

  if (is_empty_string(ns)) {
    ns = this->current_ns_;
  }

  // qualified key is constructed with
  // 3 tuples where tuples are separated
  // with '::'. The first tuple
  // is the dictionary (i.e. global, template,
  // type or others). If the first tuple is
  // either template or type, the second is the
  // qualified name for the template or type.
  // A qualified name is tuple of namespace URI
  // and local name separated by the '||'
  // character. The last tuple of a qualified key
  // is its local name and local namespace if the
  // local namespace exisited. Again, the local
  // namespace and local name is separated by '\x02'.
  // The "||" is always there even the local namespace
  // is empty.
  std::string qualified_key(dict);

  if (strcmp(dict, "template") == 0) {
    qualified_key += "::";
    qualified_key += this->current_template_;
  } else if (strcmp(dict, "type") == 0) {
    qualified_key += "::";
    qualified_key += this->current_type_;
  }
  qualified_key += "::";
  qualified_key += qualified_name(ns, key);

  auto itr = indexer_.find(qualified_key);

  if (itr != indexer_.end()) {
    if (itr->second.field_type_ == field_type) {
      itr->second.instruction_->previous_value_shared(true);
      return itr->second.storage_;
    } else
      BOOST_THROW_EXCEPTION(key_type_mismatch_error(
          qualified_key, itr->second.field_type_, field_type));
  }

  // std::cout << "adding dictionary key=" << qualified_key << "\n";
  indexer_value_type &v = indexer_[qualified_key];
  v.field_type_ = field_type;
  v.instruction_ = instruction;
  v.storage_ = candidate_storage;
  repo_base_.add_reset_entry(candidate_storage);

  return candidate_storage;
}

void dictionary_builder::visit(const int32_field_instruction *src_inst,
                               void *dest_inst) {
  int32_field_instruction *&dest =
      *static_cast<int32_field_instruction **>(dest_inst);
  dest = new (alloc_) int32_field_instruction(*src_inst);
  dest->prev_value_ =
      get_dictionary_storage(dest->name(), dest->ns(), dest->op_context_,
                             field_type_int32, &dest->prev_storage_, dest);
}

void dictionary_builder::visit(const uint32_field_instruction *src_inst,
                               void *dest_inst) {
  uint32_field_instruction *&dest =
      *static_cast<uint32_field_instruction **>(dest_inst);
  dest = new (alloc_) uint32_field_instruction(*src_inst);
  dest->prev_value_ =
      get_dictionary_storage(dest->name(), dest->ns(), dest->op_context_,
                             field_type_uint32, &dest->prev_storage_, dest);
}

void dictionary_builder::visit(const int64_field_instruction *src_inst,
                               void *dest_inst) {
  int64_field_instruction *&dest =
      *static_cast<int64_field_instruction **>(dest_inst);
  dest = new (alloc_) int64_field_instruction(*src_inst);
  dest->prev_value_ =
      get_dictionary_storage(dest->name(), dest->ns(), dest->op_context_,
                             field_type_int64, &dest->prev_storage_, dest);
}

void dictionary_builder::visit(const uint64_field_instruction *src_inst,
                               void *dest_inst) {
  uint64_field_instruction *&dest =
      *static_cast<uint64_field_instruction **>(dest_inst);
  dest = new (alloc_) uint64_field_instruction(*src_inst);
  dest->prev_value_ =
      get_dictionary_storage(dest->name(), dest->ns(), dest->op_context_,
                             field_type_uint64, &dest->prev_storage_, dest);
}

void dictionary_builder::visit(const ascii_field_instruction *src_inst,
                               void *dest_inst) {
  ascii_field_instruction *&dest =
      *static_cast<ascii_field_instruction **>(dest_inst);
  dest = new (alloc_) ascii_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(
      dest->name(), dest->ns(), dest->op_context_, field_type_ascii_string,
      &dest->prev_storage_, dest);
  repo_base_.add_vector_entry(dest->prev_value_);
}

void dictionary_builder::visit(const unicode_field_instruction *src_inst,
                               void *dest_inst) {
  unicode_field_instruction *&dest =
      *static_cast<unicode_field_instruction **>(dest_inst);
  dest = new (alloc_) unicode_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(
      dest->name(), dest->ns(), dest->op_context_, field_type_unicode_string,
      &dest->prev_storage_, dest);

  repo_base_.add_vector_entry(dest->prev_value_);
}

void dictionary_builder::visit(const decimal_field_instruction *src_inst,
                               void *dest_inst) {
  decimal_field_instruction *&dest =
      *static_cast<decimal_field_instruction **>(dest_inst);
  dest = new (alloc_) decimal_field_instruction(*src_inst);

  if (src_inst->field_type() == field_type_decimal) {
    dest->prev_value_ =
        get_dictionary_storage(dest->name(), dest->ns(), dest->op_context(),
                               field_type_decimal, &dest->prev_storage_, dest);
  } else {

    dest->mantissa_instruction_ = new (alloc_)
        mantissa_field_instruction(*src_inst->mantissa_instruction_);

    std::string mantissa_name = dest->name();
    mantissa_name += "....mantissa";
    dest->mantissa_instruction_->prev_value_ = get_dictionary_storage(
        mantissa_name.c_str(), dest->ns(),
        dest->mantissa_instruction_->op_context(), field_type_int64,
        &dest->mantissa_instruction_->prev_storage_,
        dest->mantissa_instruction_);
    std::string exponent_name = dest->name();
    exponent_name += "....exponent";
    dest->prev_value_ = get_dictionary_storage(
        exponent_name.c_str(), dest->ns(), dest->op_context(),
        field_type_exponent, &dest->prev_storage_, dest);
  }
}

void dictionary_builder::visit(const byte_vector_field_instruction *src_inst,
                               void *dest_inst) {
  byte_vector_field_instruction *&dest =
      *static_cast<byte_vector_field_instruction **>(dest_inst);
  dest = new (alloc_) byte_vector_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(
      dest->name(), dest->ns(), dest->op_context(), field_type_byte_vector,
      &dest->prev_storage_, dest);
  repo_base_.add_vector_entry(dest->prev_value_);
}

void dictionary_builder::visit(const int32_vector_field_instruction *src_inst,
                               void *dest_inst) {
  int32_vector_field_instruction *&dest =
      *static_cast<int32_vector_field_instruction **>(dest_inst);
  dest = new (alloc_) int32_vector_field_instruction(*src_inst);
}

void dictionary_builder::visit(const uint32_vector_field_instruction *src_inst,
                               void *dest_inst) {
  uint32_vector_field_instruction *&dest =
      *static_cast<uint32_vector_field_instruction **>(dest_inst);
  dest = new (alloc_) uint32_vector_field_instruction(*src_inst);
}

void dictionary_builder::visit(const int64_vector_field_instruction *src_inst,
                               void *dest_inst) {
  int64_vector_field_instruction *&dest =
      *static_cast<int64_vector_field_instruction **>(dest_inst);
  dest = new (alloc_) int64_vector_field_instruction(*src_inst);
}

void dictionary_builder::visit(const uint64_vector_field_instruction *src_inst,
                               void *dest_inst) {
  uint64_vector_field_instruction *&dest =
      *static_cast<uint64_vector_field_instruction **>(dest_inst);
  dest = new (alloc_) uint64_vector_field_instruction(*src_inst);
}

void dictionary_builder::visit(const enum_field_instruction *src_inst,
                               void *dest_inst) {
  enum_field_instruction *&dest =
      *static_cast<enum_field_instruction **>(dest_inst);
  dest = new (alloc_) enum_field_instruction(*src_inst);
  dest->prev_value_ =
      get_dictionary_storage(dest->name(), dest->ns(), dest->op_context_,
                             field_type_uint64, &dest->prev_storage_, dest);
}
}
