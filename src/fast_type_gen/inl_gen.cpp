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
#include "inl_gen.h"

void inl_gen::traverse(const mfast::group_field_instruction* inst, const char* name_suffix)
{
  std::string saved_mref_scope = mref_scope_.str();
  mref_scope_ << cpp_name(inst) << name_suffix << "_mref::";
  codegen_base::traverse(inst, name_suffix);
  reset_scope(mref_scope_, saved_mref_scope);
}

void inl_gen::gen_primitive (const char* cpp_type, const mfast::field_instruction* inst)
{
  std::string name (cpp_name(inst));
  out_ << "\n"
       << "inline\n"
       << "mfast::" << cpp_type << "_cref\n"
       << cref_scope_.str() << "get_" << name << "() const\n"
       << "{\n"
       << "  return static_cast<mfast::" << cpp_type << "_cref>((*this)[" << inst->field_index() << "]);\n"
       << "}\n\n";

  if (inst->optional() || inst->field_operator() != mfast::operator_constant) {

    out_ << "inline\n"
         << "mfast::"<< cpp_type << "_mref\n"
         << mref_scope_.str() << "set_" << name << "() const\n"
         << "{\n"
         << "  return static_cast<mfast::" << cpp_type << "_mref>((*this)[" << inst->field_index() << "]);\n"
         << "}\n";
  }
}

void inl_gen::visit(const mfast::int32_field_instruction* inst, void*)
{
  gen_primitive("int32", inst);
}

void inl_gen::visit(const mfast::uint32_field_instruction* inst, void*)
{
  gen_primitive("uint32", inst);

}

void inl_gen::visit(const mfast::int64_field_instruction* inst, void*)
{
  gen_primitive("int64", inst);
}

void inl_gen::visit(const mfast::uint64_field_instruction* inst, void*)
{
  gen_primitive("uint64", inst);
}

void inl_gen::visit(const mfast::decimal_field_instruction* inst, void*)
{
  gen_primitive("decimal", inst);
}

void inl_gen::visit(const mfast::ascii_field_instruction* inst, void*)
{
  gen_primitive("ascii_string", inst);
}

void inl_gen::visit(const mfast::unicode_field_instruction* inst, void*)
{
  gen_primitive("unicode_string", inst);
}

void inl_gen::visit(const mfast::byte_vector_field_instruction* inst, void*)
{
  gen_primitive("byte_vector", inst);
}

void inl_gen::visit(const mfast::int32_vector_field_instruction* inst, void*)
{
  gen_primitive("int32_vector", inst);
}

void inl_gen::visit(const mfast::uint32_vector_field_instruction* inst, void*)
{
  gen_primitive("uint32_vector", inst);
}

void inl_gen::visit(const mfast::int64_vector_field_instruction* inst, void*)
{
  gen_primitive("int64_vector", inst);
}

void inl_gen::visit(const mfast::uint64_vector_field_instruction* inst, void*)
{
  gen_primitive("uint64_vector", inst);
}


void inl_gen::visit(const mfast::group_field_instruction* inst, void*)
{
  std::string name (cpp_name(inst));

  std::size_t index = inst->field_index();

  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";

  bool embed_only_dyn_tempateref =
    (inst->subinstructions_count()==1) && (inst->subinstruction(0)->field_type() == mfast::field_type_templateref);

  std::stringstream cref_strm;
  std::stringstream mref_strm;

  if (embed_only_dyn_tempateref) {
    cref_strm << "mfast::aggregate_cref((*this)[" << index << "])[0]";
    mref_strm << "mfast::aggregate_mref((*this)[" << index << "])[0]";
  }
  else {
    cref_strm << "(*this)[" << index << "]";
    mref_strm << "(*this)[" << index << "]";
  }


  out_ << "\ninline " << cref_type_name << "\n"
       << cref_scope_.str() << "get_" << name << "() const\n"
       << "{\n"
       << "  return static_cast<" << cref_type_name << ">(" << cref_strm.str() << ");\n"
       << "}\n\n"
       << "inline " << mref_type_name << "\n"
       << mref_scope_.str() << "set_" << name << "() const\n"
       << "{\n"
       << "  return static_cast<" << mref_type_name << ">(" << mref_strm.str() << ");\n"
       << "}\n\n";

  if (inst->ref_template() == 0 && !embed_only_dyn_tempateref) {

    out_ << "inline\n"
         << cref_type_name << "::"<< name << "_cref(\n"
         << "  const mfast::value_storage*   storage,\n"
         << "  " << cref_type_name << "::instruction_cptr instruction)\n"
         << "  : mfast::group_cref(storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << cref_type_name << "::"<< name << "_cref(\n"
         << "  const mfast::field_cref& other)\n"
         << "  : mfast::group_cref(other)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << mref_type_name << "::"<< name << "_mref(\n"
         << "  mfast::allocator*      alloc,\n"
         << "  mfast::value_storage*  storage,\n"
         << "  " << mref_type_name << "::instruction_cptr instruction)\n"
         << "  : " << mref_type_name << "_base(alloc, storage, instruction)\n"
         << "{\n"
         << "}\n"
         << "inline\n"
         << mref_type_name << "::"<< name << "_mref(\n"
         << "  const mfast::field_mref_base& other)\n"
         << "  : " << mref_type_name << "_base(other)\n"
         << "{\n"
         << "}\n";

    traverse(inst, "");
  }
}

void inl_gen::visit(const mfast::sequence_field_instruction* inst, void*)
{
  std::string name (cpp_name(inst));

  std::size_t index = inst->field_index();

  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";

  out_ << "\ninline " << cref_type_name << "\n"
       << cref_scope_.str() << "get_" << name << "() const\n"
       << "{\n"
       << "  return static_cast<" << cref_type_name << ">((*this)[" << index << "]);\n"
       << "}\n\n"
       << "inline " << mref_type_name << "\n"
       << mref_scope_.str() << "set_" << name << "() const\n"
       << "{\n"
       << "  return static_cast<" << mref_type_name << ">((*this)[" << index << "]);\n"
       << "}\n\n";

  if (inst->ref_template() == 0  && inst->subinstructions_count() > 1) {
    out_ << "inline\n"
         << cref_scope_.str() << name << "_element_cref::"<< name << "_element_cref(\n"
         << "  const mfast::value_storage*   storage,\n"
         << "  " << cref_scope_.str() << name << "_element_cref::instruction_cptr instruction)\n"
         << "  : " << cref_scope_.str() << name << "_element_cref_base(storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << mref_scope_.str() << name << "_element_mref::"<< name << "_element_mref(\n"
         << "  mfast::allocator*               alloc,\n"
         << "  mfast::value_storage*         storage,\n"
         << "  " << mref_scope_.str() << name << "_element_mref::instruction_cptr instruction)\n"
         << "  : " << mref_scope_.str() << name << "_element_mref_base(alloc,storage, instruction)\n"
         << "{\n"
         << "}\n";

    traverse(inst, "_element");
  }
}

void inl_gen::visit(const mfast::template_instruction* inst, void*)
{
  if (inst->subinstructions_count() == 0)
    return;

  std::string name (cpp_name(inst));

  out_ << "\ninline\n"
       << name << "_cref::" << name << "_cref(\n"
       << "  const mfast::value_storage* storage_array,\n"
       << "  instruction_cptr            instruction)\n"
       << "  : mfast::aggregate_cref(storage_array, instruction)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_cref::" << name << "_cref(\n"
       << "  const mfast::message_cref& other)\n"
       << "  : mfast::aggregate_cref(other)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_cref::" << name << "_cref(\n"
       << "  const mfast::field_cref& other)\n"
       << "  : mfast::aggregate_cref(mfast::detail::field_storage_helper::storage_ptr_of(other)->of_group.content_,\n"
       << "                          static_cast<instruction_cptr>(other.instruction()))\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_mref::" << name << "_mref(\n"
       << "  mfast::allocator*     alloc,\n"
       << "  mfast::value_storage* storage_array,\n"
       << "  instruction_cptr      instruction)\n"
       << "  : " << name << "_mref_base(alloc, storage_array, instruction)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_mref::" << name << "_mref(\n"
       << "  const mfast::message_mref& other)\n"
       << "  : " << name << "_mref_base(other)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_mref::" << name << "_mref(\n"
       << "  const mfast::field_mref_base& other)\n"
       << "  : " << name << "_mref_base(other.allocator(),\n"
       << "                                  mfast::detail::field_storage_helper::storage_ptr_of(other)->of_group.content_,\n"
       << "                                  static_cast<instruction_cptr>(other.instruction()))\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "::" << name << "(\n"
       << "  mfast::allocator* alloc)\n"
       << "  : mfast::message_type(alloc, instruction(), this->data())\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "::" << name << "(\n"
       << "  const " << name << "_cref& other,\n"
       << "  mfast::allocator* alloc)\n"
       << "  : mfast::message_type(alloc, instruction(), this->data(), other.field_storage(0))\n"
       << "{\n"
       << "}\n\n"
       << "inline "<< name << "::cref_type\n"
       << name << "::ref() const\n"
       << "{\n"
       << "  return " << name << "::cref_type(my_storage_.of_group.content_, instruction());\n"
       << "}\n\n"
       << "inline "<< name << "::cref_type\n"
       << name << "::cref() const\n"
       << "{\n"
       << "  return " << name << "::cref_type(my_storage_.of_group.content_, instruction());\n"
       << "}\n\n"
       << "inline "<< name << "::mref_type\n"
       << name << "::ref()\n"
       << "{\n"
       << "  return " << name << "::mref_type(alloc_, my_storage_.of_group.content_, instruction());\n"
       << "}\n\n"
       << "inline "<< name << "::mref_type\n"
       << name << "::mref()\n"
       << "{\n"
       << "  return " << name << "::mref_type(alloc_, my_storage_.of_group.content_, instruction());\n"
       << "}\n\n";

  this->traverse(inst, "");
}

void inl_gen::visit(const mfast::templateref_instruction* inst, void*)
{
  std::size_t index = inst->field_index();

  out_ << "\n"
       << "inline\n"
       << "mfast::nested_message_cref\n"
       << cref_scope_.str() << "get_nested_message" << index << "() const\n"
       << "{\n"
       << "  return mfast::nested_message_cref((*this)[" << index << "]);\n"
       << "}\n\n";

  out_ << "inline\n"
       << "mfast::nested_message_mref\n"
       << mref_scope_.str() << "set_nested_message" << index << "() const\n"
       << "{\n"
       << "  return mfast::nested_message_mref((*this)[" << index << "]);\n"
       << "}\n";
}

void inl_gen::generate(mfast::dynamic_templates_description& desc)
{
  codegen_base::traverse(desc);
}
