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
#include "dictionary_builder.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "exceptions.h"

namespace mfast {

struct tag_key {};

typedef boost::error_info<tag_key,std::string> key_info;

struct tag_first_type {};

typedef boost::error_info<tag_first_type,std::string> first_type_info;

struct tag_second_type {};

typedef boost::error_info<tag_second_type,std::string> second_type_info;

class key_type_mismatch_error
  : public fast_dynamic_error
{
  public:
    key_type_mismatch_error(const std::string& keyname,
                            field_type_enum_t  first_type,
                            field_type_enum_t  second_type)
      : fast_dynamic_error("D4")
    {
      const char* field_type_name [] = {
        "int32",
        "uInt32",
        "int64",
        "uInt64",
        "decimal",
        "exponent",
        "ascii string",
        "unicode string",
        "byteVector"
      };
      *this << key_info(keyname)
            << first_type_info(field_type_name[first_type] )
            << second_type_info(field_type_name[second_type] );
    }

};


std::string qualified_name(const char* ns, const char* name)
{
  std::string result = ns;
  result += "||";
  result += name;
  return result;
}

dictionary_builder::dictionary_builder(dictionary_resetter& resetter, arena_allocator* allocator)
  : resetter_(resetter), alloc_(allocator)
{

}

void dictionary_builder::build(const templates_description* def)
{
  current_ns_ = def->template_ns();
  current_dictionary_ = is_empty_string(def->dictionary()) ?  "global" : def->dictionary();

  for (uint32_t i  = 0; i < def->instructions_count(); ++i ) {
    template_instruction* inst = new (*alloc_)template_instruction(*def->instruction(i));
    build_template(inst, inst);

    if (template_id_map_.count(inst->id())) {
      BOOST_THROW_EXCEPTION(duplicate_template_id_error(inst->id()) << template_name_info(inst->name()));
    }

    template_id_map_[inst->id()] = inst;

    template_name_map_[qualified_name(inst->ns(), inst->name())] = inst;
  }
}

void dictionary_builder::build_group(const field_instruction*    fi,
                                     const group_content_helper* src,
                                     group_content_helper*       dest)
{
  const char* inherited_ns = current_ns_;
  const char* inherited_dictionary = current_dictionary_;
  std::string inherited_type = current_type_;

  if (!is_empty_string(fi->ns())) {
    this->current_ns_ = fi->ns();
  }

  if (!is_empty_string(src->dictionary_)) {
    this->current_dictionary_ = src->dictionary_;
  }

  if (!is_empty_string(src->typeref_name_)) {
    const char* type_ns = this->current_ns_;
    if (!is_empty_string(src->typeref_name_)) {
      type_ns = src->typeref_ns_;
    }
    current_type_ = qualified_name(type_ns, src->typeref_name_);
  }

  std::size_t instructions_count =  src->subinstructions_count();
  field_instruction** subinstructions =
    static_cast<field_instruction**>( alloc_->allocate( instructions_count *
                                                        sizeof(field_instruction*) ) );
  for (size_t i = 0; i < instructions_count; ++i) {
    src->subinstructions_[i]->accept(*this, &subinstructions[i]);
  }

  dest->subinstructions_ = subinstructions;

  current_type_ = inherited_type;
  current_ns_ = inherited_ns;
  current_dictionary_ = inherited_dictionary;
}

void dictionary_builder::build_template(const template_instruction* src_inst, void* dest_inst)
{
  template_instruction* dest = static_cast<template_instruction*>(dest_inst);

  const char* ns = src_inst->ns();
  if (is_empty_string(ns)) {
    ns = current_ns_;
  }
  current_template_ = qualified_name(ns, src_inst->name());

  this->build_group(src_inst, src_inst, dest);
}

void dictionary_builder::visit(const template_instruction* src_inst, void* dest_inst)
{
  template_instruction*& dest = *static_cast<template_instruction**>(dest_inst);
  template_id_map_t::iterator itr = template_id_map_.find(src_inst->id());
  if (itr != template_id_map_.end()) {
    dest = itr->second;
  }
  else {
      BOOST_THROW_EXCEPTION(template_not_found_error(src_inst->name(), current_template_.c_str()));
  }
}

void dictionary_builder::visit(const templateref_instruction* src_inst, void* dest_inst)
{
  if (src_inst->name()[0] != 0)
  {
    // this is static templateRef, we have to bind to the right template instruction
    template_instruction*& dest = *static_cast<template_instruction**>(dest_inst);
    template_name_map_t::iterator itr = template_name_map_.find( qualified_name(src_inst->ns(), src_inst->name()) );
    if (itr != template_name_map_.end()) {
      dest = itr->second;
    }
    else {
      BOOST_THROW_EXCEPTION(template_not_found_error(src_inst->name(), current_template_.c_str()));
    }
  }
  else {
    // this is dynamic templateRef, it can only be binded at decoding time
    const templateref_instruction*& dest = *static_cast<const templateref_instruction**>(dest_inst);
    dest = src_inst;
  }
}

void dictionary_builder::visit(const group_field_instruction* src_inst, void* dest_inst)
{
  group_field_instruction*& dest = *static_cast<group_field_instruction**>(dest_inst);
  dest = new (*alloc_)group_field_instruction(*src_inst);
  this->build_group(src_inst, src_inst, dest);
}

void dictionary_builder::visit(const sequence_field_instruction* src_inst, void* dest_inst)
{
  sequence_field_instruction*& dest = *static_cast<sequence_field_instruction**>(dest_inst);
  dest = new (*alloc_)sequence_field_instruction(*src_inst);
  this->build_group(src_inst, src_inst, dest);
  if (src_inst->sequence_length_instruction_) {
    visit(src_inst->sequence_length_instruction_, &dest->sequence_length_instruction_);
  }
  else {
    dest->sequence_length_instruction_ = new (*alloc_)uint32_field_instruction(
      operator_none,
      presence_mandatory,
      0,
      "",
      "",
      0,
      nullable<uint32_t>()
      );
  }
}

value_storage_t*
dictionary_builder::get_dictionary_storage(const char*         key,
                                           const char*         ns,
                                           const op_context_t* op_context,
                                           field_type_enum_t   field_type,
                                           value_storage_t*    candidate_storage)
{
  const char* dict = "";

  if (op_context) {
    key = op_context->key_;
    ns = op_context->ns_;
    dict = op_context->dictionary_;
  }

  if (is_empty_string(dict)) {
    dict = this->current_dictionary_;
  }

  if (is_empty_string(ns)) {
    ns = this->current_ns_;
  }

  // qualified key is constructed with
  // 2 or 3 tuples where tuples are separated
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
  std::string qualified_key (dict);

  if (strcmp(dict, "template") == 0) {
    qualified_key += "::";
    qualified_key += this->current_template_;
  }
  else if (strcmp(dict, "type") == 0) {
    qualified_key += "::";
    qualified_key += this->current_type_;
  }
  qualified_key += "::";
  qualified_key += qualified_name(ns, key);

  indexer_t::iterator itr = indexer_.find(qualified_key);

  if (itr != indexer_.end()) {
    if (itr->second.field_type_ == field_type)
      return itr->second.storage_;
    else
      BOOST_THROW_EXCEPTION(key_type_mismatch_error(qualified_key, itr->second.field_type_, field_type));
  }

  // std::cout << "adding dictionary key=" << qualified_key << "\n";
  indexer_value_type& v = indexer_[qualified_key];
  v.field_type_ = field_type;
  v.storage_ = candidate_storage;
  resetter_.push_back(candidate_storage);

  return candidate_storage;
}

void dictionary_builder::visit(const int32_field_instruction* src_inst, void* dest_inst)
{
  int32_field_instruction*& dest = *static_cast<int32_field_instruction**>(dest_inst);
  dest = new (*alloc_)int32_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(dest->name(),
                                             dest->ns(),
                                             dest->op_context_,
                                             field_type_int32,
                                             &dest->prev_storage_);
}

void dictionary_builder::visit(const uint32_field_instruction* src_inst, void* dest_inst)
{
  uint32_field_instruction*& dest = *static_cast<uint32_field_instruction**>(dest_inst);
  dest = new (*alloc_)uint32_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(dest->name(),
                                             dest->ns(),
                                             dest->op_context_,
                                             field_type_uint32,
                                             &dest->prev_storage_);
}

void dictionary_builder::visit(const int64_field_instruction* src_inst, void* dest_inst)
{
  int64_field_instruction*& dest = *static_cast<int64_field_instruction**>(dest_inst);
  dest = new (*alloc_)int64_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(dest->name(),
                                             dest->ns(),
                                             dest->op_context_,
                                             field_type_int64,
                                             &dest->prev_storage_);
}

void dictionary_builder::visit(const uint64_field_instruction* src_inst, void* dest_inst)
{
  uint64_field_instruction*& dest = *static_cast<uint64_field_instruction**>(dest_inst);
  dest = new (*alloc_)uint64_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(dest->name(),
                                             dest->ns(),
                                             dest->op_context_,
                                             field_type_uint64,
                                             &dest->prev_storage_);
}

void dictionary_builder::visit(const ascii_field_instruction* src_inst, void* dest_inst)
{
  ascii_field_instruction*& dest = *static_cast<ascii_field_instruction**>(dest_inst);
  dest = new (*alloc_)ascii_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(dest->name(),
                                             dest->ns(),
                                             dest->op_context_,
                                             field_type_ascii_string,
                                             &dest->prev_storage_);
}

void dictionary_builder::visit(const unicode_field_instruction* src_inst, void* dest_inst)
{
  unicode_field_instruction*& dest = *static_cast<unicode_field_instruction**>(dest_inst);
  dest = new (*alloc_)unicode_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(dest->name(),
                                             dest->ns(),
                                             dest->op_context_,
                                             field_type_unicode_string,
                                             &dest->prev_storage_);
}

void dictionary_builder::visit(const decimal_field_instruction* src_inst, void* dest_inst)
{
  decimal_field_instruction*& dest = *static_cast<decimal_field_instruction**>(dest_inst);
  dest = new (*alloc_)decimal_field_instruction(*src_inst);

  if (src_inst->field_type() == field_type_decimal)
  {
    dest->prev_value_ = get_dictionary_storage(dest->name(),
                                               dest->ns(),
                                               dest->op_context(),
                                               field_type_decimal,
                                               &dest->prev_storage_);
  }
  else {

    dest->mantissa_instruction_ = new (*alloc_)mantissa_field_instruction(*src_inst->mantissa_instruction_);


    std::string mantissa_name = dest->name();
    mantissa_name += "....mantissa";
    dest->mantissa_instruction_->prev_value_ = get_dictionary_storage(mantissa_name.c_str(),
                                                                      dest->ns(),
                                                                      dest->mantissa_instruction_->op_context(),
                                                                      field_type_int64,
                                                                      &dest->mantissa_instruction_->prev_storage_);
    std::string exponent_name = dest->name();
    exponent_name += "....exponent";
    dest->prev_value_ = get_dictionary_storage(exponent_name.c_str(),
                                               dest->ns(),
                                               dest->op_context(),
                                               field_type_exponent,
                                               &dest->prev_storage_);
  }
}

void dictionary_builder::visit(const byte_vector_field_instruction* src_inst, void* dest_inst)
{
  byte_vector_field_instruction*& dest = *static_cast<byte_vector_field_instruction**>(dest_inst);
  dest = new (*alloc_)byte_vector_field_instruction(*src_inst);
  dest->prev_value_ = get_dictionary_storage(dest->name(),
                                             dest->ns(),
                                             dest->op_context(),
                                             field_type_byte_vector,
                                             &dest->prev_storage_);
}

}
