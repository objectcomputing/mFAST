// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
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

void inl_gen::gen_primitive (const char* cpp_type, const mfast::field_instruction* inst, void* pIndex)
{
  std::string name (cpp_name(inst));
  std::stringstream cref_type_name;
  cref_type_name << "mfast::" << cpp_type << "_cref";
  std::stringstream mref_type_name;
  mref_type_name << "mfast::" << cpp_type << "_mref";

  gen_accessors(inst, name, cref_type_name.str(), mref_type_name.str(), pIndex);
}

void inl_gen::visit(const mfast::int32_field_instruction* inst, void* pIndex)
{
  gen_primitive("int32", inst, pIndex);
}

void inl_gen::visit(const mfast::uint32_field_instruction* inst, void* pIndex)
{
  gen_primitive("uint32", inst, pIndex);

}

void inl_gen::visit(const mfast::int64_field_instruction* inst, void* pIndex)
{
  gen_primitive("int64", inst, pIndex);
}

void inl_gen::visit(const mfast::uint64_field_instruction* inst, void* pIndex)
{
  gen_primitive("uint64", inst, pIndex);
}

void inl_gen::visit(const mfast::decimal_field_instruction* inst, void* pIndex)
{
  gen_primitive("decimal", inst, pIndex);
}

void inl_gen::visit(const mfast::ascii_field_instruction* inst, void* pIndex)
{
  gen_primitive("ascii_string", inst, pIndex);
}

void inl_gen::visit(const mfast::unicode_field_instruction* inst, void* pIndex)
{
  gen_primitive("unicode_string", inst, pIndex);
}

void inl_gen::visit(const mfast::byte_vector_field_instruction* inst, void* pIndex)
{
  gen_primitive("byte_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::int32_vector_field_instruction* inst, void* pIndex)
{
  gen_primitive("int32_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::uint32_vector_field_instruction* inst, void* pIndex)
{
  gen_primitive("uint32_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::int64_vector_field_instruction* inst, void* pIndex)
{
  gen_primitive("int64_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::uint64_vector_field_instruction* inst, void* pIndex)
{
  gen_primitive("uint64_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::group_field_instruction* inst, void* pIndex)
{
  std::string name (cpp_name(inst));

  std::size_t index = pIndex == 0 ? 0 : *static_cast<std::size_t*>(pIndex);

  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";

  bool embed_only_dyn_tempateref = contains_only_templateref(inst);

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

  if (pIndex) {
    out_ << "\ninline " << cref_type_name << "\n"
         << cref_scope_.str() << "get_" << name << "() const\n"
         << "{\n";
    if (inst->optional())
      out_ << "  if (" << "(*this)[" << index << "].absent())\n"
           << "    return " << cref_type_name << "(0, " << cref_type_name << "::instruction_cptr(0));\n";
    out_ << "  return static_cast<" << cref_type_name << ">(" << cref_strm.str() << ");\n"
         << "}\n\n"
         << "\ninline " << cref_type_name << "\n"
         << cref_scope_.str() << "try_get_" << name << "() const\n"
         << "{\n";
    if (inst->optional())
      out_ << "  if (" << "(*this)[" << index << "].absent())\n"
           << "    throw mfast::bad_optional_access();\n";
    out_ << "  return static_cast<" << cref_type_name << ">(" << cref_strm.str() << ");\n"
         << "}\n\n"
         << "inline " << mref_type_name << "\n"
         << mref_scope_.str() << "set_" << name << "() const\n"
         << "{\n";
    if (inst->optional())
      out_ << "  this->field_storage(" << index << ")->present(true);\n";
    out_ << "  return " << mref_type_name << "("<< mref_strm.str() << ");\n"
         << "}\n\n";

    if (inst->optional()) {
      out_ << "inline\n"
           << "void\n"
           << mref_scope_.str() << "omit_" << name << "() const\n"
           << "{\n"
           << "  (*this)[" << index << "].omit();\n"
           << "}\n\n";
    }

    if (inst->ref_instruction()) {
      out_ << "inline void\n"
           << mref_scope_.str() << "link_" << name << "(const " << mref_type_name << "& ref) const\n"
           << "{\n"
           << "  this->link_group_at(" << index << ", ref);\n"
           << "}\n\n"
           << "inline void\n"
           << mref_scope_.str() << "unlink_" << name << "() const\n"
           << "{\n"
           << "  this->unlink_group_at(" << index << ");\n"
           << "}\n\n";
    }
  }

  if (inst->ref_instruction() == 0 && !embed_only_dyn_tempateref) {

    out_ << "inline\n"
         << cref_type_name << "::"<< name << "_cref()\n"
         << "  : base_type(0, 0)\n"
         << "{\n"
         << "}\n\n"
         << "template <typename T>"
         << "inline\n"
         << cref_type_name << "::" << name << "_cref(\n"
         << "  typename boost::enable_if<boost::is_same<typename T::cref_type, " << name << "_cref>, const mfast::value_storage*>::type storage_array,\n"
         << "  const T* instruction)\n"
         << "  : base_type(storage_array, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << cref_type_name << "::"<< name << "_cref(\n"
         << "  const mfast::field_cref& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << mref_type_name << "::"<< name << "_mref()\n"
         << "  : base_type(0, 0, 0)\n"
         << "{\n"
         << "}\n\n"
         << "template <typename T>"
         << "inline\n"
         << mref_type_name << "::" << name << "_mref(\n"
         << "  mfast::allocator* alloc,\n"
         << "  typename boost::enable_if<boost::is_same<typename T::cref_type, " << name << "_cref>, mfast::value_storage*>::type storage_array,\n"
         << "  const T*          instruction)\n"
         << "  : base_type(alloc, storage_array, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << mref_type_name << "::"<< name << "_mref(\n"
         << "  const mfast::field_mref_base& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n";

    if (!pIndex)
    {
      out_ << "inline\n"
           << name << "::" << name << "(\n"
           << "  mfast::allocator* alloc)\n"
           << "  : base_type(alloc, instruction(), this->data())\n"
           << "{\n"
           << "}\n\n"
           << "inline\n"
           << name << "::" << name << "(\n"
           << "  const " << name << "_cref& other,\n"
           << "  mfast::allocator* alloc)\n"
           << "  : base_type(alloc, instruction(), this->data(), other.field_storage(0))\n"
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
    }

    traverse(inst, "");
  }
}

void inl_gen::visit(const mfast::sequence_field_instruction* inst, void* pIndex)
{
  std::string name (cpp_name(inst));

  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";

  gen_accessors(inst, name, cref_type_name, mref_type_name, pIndex);

  const mfast::field_instruction* element_instruction = get_element_instruction(inst);

  if (inst->ref_instruction() == 0 && element_instruction==0) {
    out_ << "inline\n"
         << cref_scope_.str() << name << "_element_cref::"<< name << "_element_cref(\n"
         << "  const mfast::value_storage*   storage,\n"
         << "  " << cref_scope_.str() << name << "_element_cref::instruction_cptr instruction)\n"
         << "  : base_type(storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << mref_scope_.str() << name << "_element_mref::"<< name << "_element_mref(\n"
         << "  mfast::allocator*               alloc,\n"
         << "  mfast::value_storage*         storage,\n"
         << "  " << mref_scope_.str() << name << "_element_mref::instruction_cptr instruction)\n"
         << "  : base_type(alloc,storage, instruction)\n"
         << "{\n"
         << "}\n\n";

    traverse(inst, "_element");
  }

  if (pIndex == 0)
  {
    out_ << "inline\n"
         << name << "::" << name << "(\n"
         << "  mfast::allocator* alloc)\n"
         << "  : base_type(alloc, instruction(), 0)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << name << "::" << name << "(\n"
         << "  const " << name << "_cref& other,\n"
         << "  mfast::allocator* alloc)\n"
         << "  : base_type(reinterpret_cast<const mfast::sequence_cref&>(other), alloc)\n"
         << "{\n"
         << "}\n\n"
         << "inline "<< name << "::cref_type\n"
         << name << "::ref() const\n"
         << "{\n"
         << "  return " << name << "::cref_type(&my_storage_, instruction());\n"
         << "}\n\n"
         << "inline "<< name << "::cref_type\n"
         << name << "::cref() const\n"
         << "{\n"
         << "  return " << name << "::cref_type(&my_storage_, instruction());\n"
         << "}\n\n"
         << "inline "<< name << "::mref_type\n"
         << name << "::ref()\n"
         << "{\n"
         << "  return " << name << "::mref_type(alloc_, &my_storage_, instruction());\n"
         << "}\n\n"
         << "inline "<< name << "::mref_type\n"
         << name << "::mref()\n"
         << "{\n"
         << "  return " << name << "::mref_type(alloc_, &my_storage_, instruction());\n"
         << "}\n\n";
  }

}

void inl_gen::visit(const mfast::template_instruction* inst, void*)
{
  // if (inst->subinstructions().size() == 0)
  //   return;

  std::string name (cpp_name(inst));

  out_ << "inline\n"
       << name << "_cref::" << name << "_cref()\n"
       << "  : base_type(0, 0)\n"
       << "{\n"
       << "}\n\n"
       << "template <typename T>"
       << "inline\n"
       << name << "_cref::" << name << "_cref(\n"
       << "  typename boost::enable_if<boost::is_same<typename T::cref_type, " << name << "_cref>, const mfast::value_storage*>::type storage_array,\n"
       << "  const T* instruction)\n"
       << "  : base_type(storage_array, instruction)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_cref::" << name << "_cref(\n"
       << "  const mfast::message_cref& other)\n"
       << "  : base_type(other)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_cref::" << name << "_cref(\n"
       << "  const mfast::field_cref& other)\n"
       << "  : base_type(mfast::field_cref_core_access::storage_of(other)->of_group.content_,\n"
       << "              static_cast<instruction_cptr>(other.instruction()))\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_mref::" << name << "_mref()\n"
       << "  : base_type(0, 0, 0)\n"
       << "{\n"
       << "}\n\n"
       << "template <typename T>"
       << "inline\n"
       << name << "_mref::" << name << "_mref(\n"
       << "  mfast::allocator* alloc,\n"
       << "  typename boost::enable_if<boost::is_same<typename T::cref_type, " << name << "_cref>, mfast::value_storage*>::type storage_array,\n"
       << "  const T*          instruction)\n"
       << "  : base_type(alloc, storage_array, instruction)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_mref::" << name << "_mref(\n"
       << "  const mfast::message_mref& other)\n"
       << "  : base_type(other)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "_mref::" << name << "_mref(\n"
       << "  const mfast::field_mref_base& other)\n"
       << "  : base_type(other.allocator(),\n"
       << "              mfast::field_mref_core_access::storage_of(other)->of_group.content_,\n"
       << "              static_cast<instruction_cptr>(other.instruction()))\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "::" << name << "(\n"
       << "  mfast::allocator* alloc)\n"
       << "  : base_type(alloc, instruction(), this->data())\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name << "::" << name << "(\n"
       << "  const " << name << "_cref& other,\n"
       << "  mfast::allocator* alloc)\n"
       << "  : base_type(alloc, instruction(), this->data(), other.field_storage(0))\n"
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

void inl_gen::visit(const mfast::templateref_instruction* , void* pIndex)
{
  std::size_t index = *static_cast<std::size_t*>(pIndex);

  out_ << "\n"
       << "inline\n"
       << "mfast::nested_message_cref\n"
       << cref_scope_.str() << "get_nested_message" << index << "() const\n"
       << "{\n"
       << "  return mfast::nested_message_cref((*this)[" << index << "]);\n"
       << "}\n\n"
       << cref_scope_.str() << "try_get_nested_message" << index << "() const\n"
       << "{\n" << "  return mfast::nested_message_cref((*this)[" << index << "]);\n"
       << "}\n\n"
       << "inline\n"
       << "mfast::nested_message_mref\n"
       << mref_scope_.str() << "set_nested_message" << index << "() const\n"
       << "{\n"
       << "  return mfast::nested_message_mref((*this)[" << index << "]);\n"
       << "}\n\n";
}

void inl_gen::generate(mfast::dynamic_templates_description& desc)
{
  codegen_base::traverse(desc);
  BOOST_FOREACH(const mfast::aggregate_view_info& info, desc.view_infos())
  {
    this->generate(info);
  }
}

void inl_gen::visit(const mfast::enum_field_instruction* inst, void* pIndex)
{
  std::string name (cpp_name(inst));
  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";


  if (inst->ref_instruction() == 0) {

    out_ << "inline\n"
         << cref_type_name << "::"<< name << "_cref(\n"
         << "  const mfast::value_storage*   storage,\n"
         << "  " << cref_type_name << "::instruction_cptr instruction)\n"
         << "  : base_type(storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << cref_type_name << "::"<< name << "_cref(\n"
         << "  const mfast::field_cref& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << mref_type_name << "::"<< name << "_mref(\n"
         << "  mfast::allocator*      alloc,\n"
         << "  mfast::value_storage*  storage,\n"
         << "  " << mref_type_name << "::instruction_cptr instruction)\n"
         << "  : base_type(alloc, storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << mref_type_name << "::"<< name << "_mref(\n"
         << "  const mfast::field_mref_base& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n"
         << cref_type_name << "::element_type\n"
         << cref_type_name << "::value() const\n"
         << "{\n"
         << "  return static_cast<" << name << "::element>(base_type::value());\n"
         << "}\n\n";


    for (uint64_t i = 0; i < inst->num_elements_; ++i)
    {

      std::string element_name = cpp_name(inst->elements_[i]);

      out_ << "inline\n"
           << "bool " << cref_type_name << "::is_" << element_name << "() const\n"
           << "{\n"
           << "  return this->value() == " << name << "::" <<  element_name << ";\n"
           << "}\n\n"
           << "inline\n"
           << "void " << mref_type_name << "::as_" << element_name << "() const\n"
           << "{\n"
           << "  return this->as(" << name << "::" <<  element_name << ");\n"
           << "}\n\n";

    }
  }


  gen_accessors(inst, name, cref_type_name, mref_type_name, pIndex);
}

void inl_gen::gen_accessors(const mfast::field_instruction* inst,
                            const std::string&              name,
                            const std::string&              cref_type_name,
                            const std::string&              mref_type_name,
                            void*                           pIndex)
{
  if (pIndex) {
    std::size_t index = *static_cast<std::size_t*>(pIndex);
    out_ << "inline " << cref_type_name << "\n"
         << cref_scope_.str() << "get_" << name << "() const\n"
         << "{\n"
         << "  return static_cast<" << cref_type_name << ">((*this)[" << index << "]);\n"
         << "}\n\n";

    out_ << "inline " << cref_type_name << "\n"
         << cref_scope_.str() << "try_get_" << name << "() const\n"
         << "{\n";

   if (inst->optional())
     out_ << "  if (" << "(*this)[" << index << "].absent())\n"
          << "    throw mfast::bad_optional_access();\n";

    out_ << "  return static_cast<" << cref_type_name << ">((*this)[" << index << "]);\n"
         << "}\n\n";

    if (!is_const_field(inst)) {
      out_ << "inline " << mref_type_name << "\n"
           << mref_scope_.str() << "set_" << name << "() const\n"
           << "{\n"
           << "  return static_cast<" << mref_type_name << ">((*this)[" << index << "]);\n"
           << "}\n\n";
    }
    if (inst->optional()) {
      out_ << "inline\n"
           << "void\n"
           << mref_scope_.str() << "omit_" << name << "() const\n"
           << "{\n"
           << "  (*this)[" << index << "].omit();\n"
           << "}\n\n";
    }
  }
}

void inl_gen::generate(const mfast::aggregate_view_info& info)
{
  std::string ns_prefix;
  std::string my_name = cpp_name(info.name_);

  if (this->cpp_ns_ != info.instruction_->cpp_ns())
  {
    ns_prefix = info.instruction_->cpp_ns();
    ns_prefix += "::";
  }

  out_ << "inline " << my_name << "::"<< my_name << "(const " << ns_prefix <<  cpp_name(info.instruction_->name()) << "_cref& ref)\n"
       << "  : ref_(ref)\n"
       << "{\n"
       << "}\n\n"
       << "inline mfast::view_iterator " << my_name << "::begin() const\n"
       << "{\n"
       << "  return mfast::view_iterator(ref_, info_.data_.begin(), info_.max_depth_);\n"
       << "}\n\n"
       << "inline mfast::view_iterator  " << my_name << "::end() const\n"
       << "{\n"
       << "  return mfast::view_iterator(info_.data_.end()-1);\n"
       << "}\n\n"
       << "template <typename FieldAccessor>\n"
       << "inline void  " << my_name << "::accept_accessor(FieldAccessor& accessor) const\n"
       << "{\n"
       << "  BOOST_FOREACH(mfast::field_cref f, *this)\n"
       << "  {\n"
       << "    if (FieldAccessor::visit_absent || f.present())"
       << "      f.accept_accessor(accessor);\n"
       << "  }\n"
       << "}\n";
}
