// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "hpp_gen.h"
#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#endif
#include <boost/algorithm/string.hpp>
#if defined __GNUC__
#pragma GCC diagnostic pop
#endif
indent_t indent;

void hpp_gen::set_export_symbol(const char *symbol) {
  export_symbol_ = symbol;
  export_symbol_uppercase_ = boost::to_upper_copy(export_symbol_);
  export_symbol_uppercase_ += " ";
}

void hpp_gen::traverse(const mfast::group_field_instruction *inst,
                       const char *name_suffix) {
  header_cref_.inc_indent(2);
  header_mref_.inc_indent(2);
  codegen_base::traverse(inst, name_suffix);
  header_cref_.dec_indent(2);
  header_mref_.dec_indent(2);
}

void hpp_gen::gen_primitive(const char *cpp_type,
                            const mfast::field_instruction *inst) {
  std::string name(cpp_name(inst));
  header_cref_ << indent << "mfast::" << cpp_type << "_cref get_" << name
               << "() const;\n";
  header_cref_ << indent << "mfast::" << cpp_type << "_cref try_get_" << name
               << "() const;\n";
  if (!is_const_field(inst)) {
    header_mref_ << indent << "mfast::" << cpp_type << "_mref set_" << name
                 << "() const;\n";
  }

  if (inst->optional()) {
    header_mref_ << indent << "void omit_" << name << "() const;\n";
  }
}

void hpp_gen::visit(const mfast::int32_field_instruction *inst, void *) {
  gen_primitive("int32", inst);
}

void hpp_gen::visit(const mfast::uint32_field_instruction *inst, void *) {
  gen_primitive("uint32", inst);
}

void hpp_gen::visit(const mfast::int64_field_instruction *inst, void *) {
  gen_primitive("int64", inst);
}

void hpp_gen::visit(const mfast::uint64_field_instruction *inst, void *) {
  gen_primitive("uint64", inst);
}

void hpp_gen::visit(const mfast::decimal_field_instruction *inst, void *) {
  gen_primitive("decimal", inst);
}

void hpp_gen::visit(const mfast::ascii_field_instruction *inst, void *) {
  gen_primitive("ascii_string", inst);
}

void hpp_gen::visit(const mfast::unicode_field_instruction *inst, void *) {
  gen_primitive("unicode_string", inst);
}

void hpp_gen::visit(const mfast::byte_vector_field_instruction *inst, void *) {
  gen_primitive("byte_vector", inst);
}

void hpp_gen::visit(const mfast::int32_vector_field_instruction *inst, void *) {
  gen_primitive("int32_vector", inst);
}

void hpp_gen::visit(const mfast::uint32_vector_field_instruction *inst,
                    void *) {
  gen_primitive("uint32_vector", inst);
}

void hpp_gen::visit(const mfast::int64_vector_field_instruction *inst, void *) {
  gen_primitive("int64_vector", inst);
}

void hpp_gen::visit(const mfast::uint64_vector_field_instruction *inst,
                    void *) {
  gen_primitive("uint64_vector", inst);
}

void hpp_gen::visit(const mfast::group_field_instruction *inst, void *pIndex) {
  std::string name(cpp_name(inst));

  std::string cpp_type;

  if (inst->ref_instruction()) {
    cpp_type = cpp_type_of(inst, &dependency_);
  } else if (this->contains_only_templateref(inst)) {
    cpp_type = "mfast::nested_message";
  }

  if (cpp_type.size()) {

    if (pIndex) {
      header_cref_ << indent << "typedef " << cpp_type << "_cref " << name
                   << "_cref;\n";
      header_mref_ << indent << "typedef " << cpp_type << "_mref " << name
                   << "_mref;\n";
      header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
      header_cref_ << indent << name << "_cref try_get_" << name
                   << "() const;\n";
      header_mref_ << indent << name << "_mref set_" << name << "() const;\n";

      if (inst->optional()) {
        header_mref_ << indent << "void omit_" << name << "() const;\n";
      }

      header_mref_ << indent << "void link_" << name << "(const " << name
                   << "_mref& ref) const;\n" << indent << "void unlink_" << name
                   << "() const;\n";
    }
  } else {

    header_cref_
        << "\n" << indent << "class " << name << "_cref\n" << indent
        << "  : public mfast::aggregate_cref\n" << indent << "{\n" << indent
        << "  typedef mfast::aggregate_cref base_type;\n" << indent
        << "  public:\n" << indent << "    typedef mfast::group_instruction_ex<"
        << name << "_cref> instruction_type;\n\n" << indent
        << "    typedef const instruction_type* instruction_cptr;\n" << indent
        << "    " << name << "_cref();\n" << indent
        << "    template <typename T>\n" << indent << "    " << name
        << "_cref(\n" << indent
        << "      typename std::enable_if<std::is_same<typename T::cref_type, "
        << name
        << "_cref>::value, const mfast::value_storage*>::type storage,\n"
        << indent << "      const T* instruction);\n\n" << indent
        << "    explicit " << name
        << "_cref(const mfast::field_cref& other);\n\n" << indent
        << "    template <typename Visitor>\n" << indent
        << "    void accept(Visitor& v);\n\n";

    std::stringstream mref_base_type;
    mref_base_type << "mfast::make_aggregate_mref<" << cref_scope_.str() << name
                   << "_cref>";

    header_mref_
        << "\n" << indent << "class " << name << "_mref\n" << indent
        << "  : public " << mref_base_type.str() << "\n" << indent << "{\n"
        << indent << "  typedef " << mref_base_type.str() << " base_type;\n"
        << indent << "  public:\n" << indent << "    " << name << "_mref();\n"
        << indent << "    template <typename T>\n" << indent << "    " << name
        << "_mref(\n" << indent << "      mfast::allocator*       alloc,\n"
        << indent
        << "      typename std::enable_if<std::is_same<typename T::cref_type, "
        << name << "_cref>::value, mfast::value_storage*>::type   storage,\n"
        << indent << "      const T* instruction);\n\n" << indent
        << "    explicit " << name
        << "_mref(const mfast::field_mref_base& other);\n\n" << indent
        << "    template <typename Visitor>\n" << indent
        << "    void accept(Visitor& v);\n\n";

    this->traverse(inst, "");

    header_cref_ << indent << "};\n\n";
    header_mref_ << indent << "};\n\n";
    if (pIndex) {
      header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
      header_cref_ << indent << name << "_cref try_get_" << name
                   << "() const;\n";
      header_mref_ << indent << name << "_mref set_" << name << "() const;\n";
      if (inst->optional()) {
        header_mref_ << indent << "void omit_" << name << "() const;\n";
      }
    }
  }

  if (pIndex == nullptr) {
    content_ << header_cref_.str() << header_mref_.str();

    header_cref_.clear();
    header_cref_.str("");
    header_mref_.clear();
    header_mref_.str("");

    content_ << "class ";

    content_ << export_symbol_uppercase_ << name << "\n"
             << "  : private std::array<mfast::value_storage, "
             << inst->subinstructions().size() << ">\n"
             << "  , public mfast::group_type\n"
             << "{\n"
             << "  private:\n"
             << "    typedef  std::array<mfast::value_storage, "
             << inst->subinstructions().size() << "> base_array;\n"
             << "    typedef mfast::group_type base_type;\n"
             << "  public:\n"
             << "    typedef mfast::group_instruction_ex<" << name
             << "_cref> instruction_type;\n\n"
             << "    typedef " << name << "_cref cref_type;\n"
             << "    typedef " << name << "_mref mref_type;\n"
             << "    " << name << "(\n"
             << "      mfast::allocator* "
                "alloc=mfast::malloc_allocator::instance());\n"
             << "    "
             << name
             << "(\n"
             << "      const "
             << name
             << "_cref& other,\n"
             << "      mfast::allocator* "
                "alloc=mfast::malloc_allocator::instance());\n"
             << "    cref_type ref() const;\n"
             << "    cref_type cref() const;\n"
             << "    mref_type ref();\n"
             << "    mref_type mref();\n"
             << "    static const instruction_type* instruction();\n"
             << "\n"
             << "    "
             << name
             << "("
             << name
             << "&&) BOOST_NOEXCEPT;\n"
             << "    "
             << name
             << "(const "
             << name
             << "&);\n"
             << "    "
             << name
             << "& operator = ("
             << name
             << "&&) BOOST_NOEXCEPT;\n"
             << "    "
             << name
             << "& operator = (const "
             << name
             << "&);\n"
             << "};\n\n";
  }
}

void hpp_gen::gen_sequence_typedef(
    const mfast::sequence_field_instruction *inst, const std::string &name) {
  std::string element_type;
  std::string trait;
  const mfast::field_instruction *element_instruction =
      get_element_instruction(inst);
  element_type = cpp_type_of(element_instruction, &dependency_);

  if (inst->element_instruction())
    trait = "mfast::defined_element_sequence_trait";
  else {
    trait = "mfast::sole_element_sequence_trait";
  }

  if (element_type.size() == 0) {
    // in this case, it's most likely to be a sequence of sequnece
    const mfast::sequence_field_instruction *nested_inst =
        dynamic_cast<const mfast::sequence_field_instruction *>(
            element_instruction);
    if (nested_inst) {
      element_type = name + "_element";
      gen_sequence_typedef(nested_inst, element_type);
    }
  }
  header_cref_ << indent << "typedef mfast::make_sequence_cref<" << element_type
               << "_cref, " << trait << "> " << name << "_cref;\n";
  header_mref_ << indent << "typedef mfast::make_sequence_mref<" << element_type
               << "_mref, " << trait << "> " << name << "_mref;\n";
}

void hpp_gen::visit(const mfast::sequence_field_instruction *inst,
                    void *pIndex) {
  std::string name(cpp_name(inst));
  const mfast::field_instruction *element_instruction =
      get_element_instruction(inst);

  if (inst->ref_instruction()) // make sure this is not a static templateRef
  {
    std::string cpp_type = cpp_type_of(inst, &dependency_);

    if (inst->ref_instruction()->field_type() != mfast::field_type_template) {

      header_cref_ << indent << "typedef " << cpp_type << "_cref " << name
                   << "_cref;\n";
      header_mref_ << indent << "typedef " << cpp_type << "_mref " << name
                   << "_mref;\n";
    } else {
      header_cref_ << indent << "typedef " << cpp_type << "_cref::" << name
                   << "_cref " << name << "_cref;\n";
      header_mref_ << indent << "typedef " << cpp_type << "_mref::" << name
                   << "_mref " << name << "_mref;\n";
    }

  } else if (element_instruction == nullptr) {
    std::string element_type = name + "_element";

    header_cref_ << "\n" << indent << "class " << element_type << "_cref\n"
                 << indent << "  : public mfast::sequence_element_cref\n"
                 << indent << "{\n" << indent
                 << "  typedef mfast::sequence_element_cref base_type;\n"
                 << indent << "  public:\n" << indent
                 << "    typedef mfast::sequence_instruction_ex<"
                 << element_type << "_cref> instruction_type;\n" << indent
                 << "    typedef const instruction_type* instruction_cptr;\n"
                 << indent << "    " << element_type << "_cref(\n" << indent
                 << "      const mfast::value_storage* storage,\n" << indent
                 << "      instruction_cptr            instruction);\n\n"
                 << indent << "    template <typename Visitor>\n" << indent
                 << "    void accept(Visitor& v);\n\n";

    header_mref_ << "\n" << indent << "class " << element_type << "_mref\n"
                 << indent << "  : public mfast::make_aggregate_mref<"
                 << cref_scope_.str() << element_type << "_cref>\n" << indent
                 << "{\n" << indent << "  typedef mfast::make_aggregate_mref<"
                 << cref_scope_.str() << element_type << "_cref> base_type;\n"
                 << indent << "  public:\n" << indent << "    " << element_type
                 << "_mref(\n" << indent
                 << "      mfast::allocator*     alloc,\n" << indent
                 << "      mfast::value_storage* storage,\n" << indent
                 << "      instruction_cptr      instruction);\n\n" << indent
                 << "    template <typename Visitor>\n" << indent
                 << "    void accept(Visitor& v);\n\n";

    this->traverse(inst, "_element");

    header_cref_ << indent << "};\n\n";
    header_mref_ << indent << "};\n\n";

    header_cref_ << indent << "typedef mfast::make_sequence_cref<"
                 << element_type << "_cref> " << name << "_cref;\n";
    header_mref_ << indent << "typedef mfast::make_sequence_mref<"
                 << element_type << "_mref> " << name << "_mref;\n";

  } else {
    gen_sequence_typedef(inst, name);
  }

  if (pIndex) {
    header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
    header_cref_ << indent << name << "_cref try_get_" << name << "() const;\n";
    header_mref_ << indent << name << "_mref set_" << name << "() const;\n";
    if (inst->optional())
      header_mref_ << indent << "void omit_" << name << "() const;\n";
  }

  if (pIndex == nullptr) {
    content_ << header_cref_.str() << header_mref_.str();

    header_cref_.clear();
    header_cref_.str("");
    header_mref_.clear();
    header_mref_.str("");
    content_ << "class ";

    content_ << export_symbol_uppercase_ << name << "\n"
             << "  : public mfast::sequence_type\n"
             << "{\n"
             << "  typedef mfast::sequence_type base_type;\n"
             << "  public:\n"
             << "    typedef " << name
             << "_cref::instruction_type instruction_type;\n\n"
             << "    typedef " << name << "_cref cref_type;\n"
             << "    typedef " << name << "_mref mref_type;\n"
             << "    " << name << "(\n"
             << "      mfast::allocator* "
                "alloc=mfast::malloc_allocator::instance());\n"
             << "    "
             << name
             << "(\n"
             << "      const "
             << name
             << "_cref& other,\n"
             << "      mfast::allocator* "
                "alloc=mfast::malloc_allocator::instance());\n"
             << "    cref_type ref() const;\n"
             << "    cref_type cref() const;\n"
             << "    mref_type ref();\n"
             << "    mref_type mref();\n"
             << "    static const instruction_type* instruction();\n"
             << "  private:\n"
             << "    "
             << name
             << "(const "
             << name
             << "&);\n"
             << "    "
             << name
             << "& operator = (const "
             << name
             << "&);\n"
             << "};\n\n";
  }
}

void hpp_gen::visit(const mfast::template_instruction *inst, void *) {
  // if (inst->subinstructions().size() == 0)
  //   return;

  std::string name(cpp_name(inst));
  header_cref_
      << "\n" << indent << "class " << name << "_cref\n" << indent
      << "  : public mfast::aggregate_cref\n" << indent << "{\n" << indent
      << "  typedef mfast::aggregate_cref base_type;\n" << indent
      << "  public:\n" << indent
      << "    typedef mfast::template_instruction_ex<" << name
      << "_cref> instruction_type;\n" << indent
      << "    typedef const instruction_type* instruction_cptr;\n" << indent
      << "    " << name << "_cref();\n" << indent
      << "    template <typename T>\n" << indent << "    " << name << "_cref(\n"
      << indent
      << "      typename std::enable_if<std::is_same<typename T::cref_type, "
      << name << "_cref>::value, const mfast::value_storage*>::type storage,\n"
      << indent << "      const T* instruction);\n\n" << indent << "    "
      << name << "_cref(const mfast::message_cref& other);\n\n" << indent
      << "    explicit " << name << "_cref(const mfast::field_cref& other);\n\n"
      << indent << "    operator mfast::message_cref();\n" << indent
      << "    template <typename Visitor>\n" << indent
      << "    void accept(Visitor& v);\n\n";

  header_mref_
      << "\n" << indent << "typedef mfast::make_aggregate_mref<" << name
      << "_cref> " << name << "_mref_base;\n" << indent << "class " << name
      << "_mref\n" << indent << "  : public " << name << "_mref_base\n"
      << indent << "{\n" << indent << "  typedef " << name
      << "_mref_base base_type;\n" << indent << "  public:\n" << indent
      << "    typedef mfast::template_instruction_ex<" << name
      << "_cref> instruction_type;\n" << indent
      << "    typedef const instruction_type* instruction_cptr;\n" << indent
      << "    " << name << "_mref();\n" << indent
      << "    template <typename T>\n" << indent << "    " << name << "_mref(\n"
      << indent << "      mfast::allocator*       alloc,\n" << indent
      << "      typename std::enable_if<std::is_same<typename T::cref_type, "
      << name << "_cref>::value, mfast::value_storage*>::type   storage,\n"
      << indent << "      const T* instruction);\n\n" << indent << "    "
      << name << "_mref(const mfast::message_mref& other);\n\n" << indent
      << "    operator mfast::message_mref();\n" << indent << "    explicit "
      << name << "_mref(const mfast::field_mref_base& other);\n\n" << indent
      << "    template <typename Visitor>\n" << indent
      << "    void accept(Visitor& v);\n\n";

  this->traverse(inst, "");

  header_cref_ << "\n" << indent << "};\n\n";
  header_mref_ << indent << "};\n\n";

  content_ << header_cref_.str() << header_mref_.str();

  header_cref_.clear();
  header_cref_.str("");
  header_mref_.clear();
  header_mref_.str("");

  content_ << "class ";

  content_
      << export_symbol_uppercase_ << name << "\n"
      << "  : private mfast::value_storage_array<"
      << inst->subinstructions().size() << ">\n"
      << "  , public mfast::message_type\n"
      << "{\n"
      << "  typedef mfast::message_type base_type;\n"
      << "  public:\n"
      << "    enum {\n"
      << "      the_id = " << inst->id() << "\n"
      << "    };\n"
      << "    typedef mfast::template_instruction_ex<" << name
      << "_cref> instruction_type;\n\n"
      << "    typedef mfast::make_message_cref<" << name
      << "_cref, instruction_type> cref_type;\n"
      << "    typedef mfast::make_message_mref<" << name
      << "_mref, instruction_type> mref_type;\n"
      << "    " << name << "(\n"
      << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
      << "    " << name << "(\n"
      << "      const " << name << "_cref& other,\n"
      << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
      << "#ifdef MFAST_JSON_H\n"
      << "    explicit " << name << "(\n"
      << "      const char* json_initializer,\n"
      << "      int json_object_tag_mask=0,\n"
      << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
      << "#endif // MFAST_JSON_H\n"
      << "    cref_type ref() const;\n"
      << "    cref_type cref() const;\n"
      << "    mref_type ref();\n"
      << "    mref_type mref();\n"
      << "    static const instruction_type* instruction();\n"
      << "  private:\n"
      << "    " << name << "(const " << name << "&);\n"
      << "    " << name << "& operator = (const " << name << "&);\n"
      << "};\n\n";
}

void hpp_gen::visit(const mfast::templateref_instruction *, void *pIndex) {
  std::size_t index = *static_cast<std::size_t *>(pIndex);
  header_cref_ << indent << "mfast::nested_message_cref get_nested_message"
               << index << "() const;\n";
  header_cref_ << indent << "mfast::nested_message_cref try_get_nested_message"
               << index << "() const;\n";
  header_mref_ << indent << "mfast::nested_message_mref set_nested_message"
               << index << "() const;\n";
}

void hpp_gen::generate(mfast::dynamic_templates_description &desc) {
  codegen_base::traverse(desc);

  std::string filebase_upper = boost::to_upper_copy(filebase_);

  generate_comment();

  out_ << "#pragma once\n"
       << "\n"
       << "#include <mfast.h>\n";
  // << "#include <boost/array.hpp>\n";

  for (const std::string &dep : dependency_) {
    if (dep != "mfast")
      out_ << "#include \"" << dep << hpp_fileext_ << "\"\n";
  }

  if (export_symbol_.size()) {
    out_ << "#include \"" << export_symbol_ << ".h\"\n";
  }

  for (auto &&ns : outer_ns_)
    out_ << "namespace " << ns << "\n{\n";
  out_ << "namespace " << filebase_ << "\n{\n" << content_.str() << "\n";

  for (const mfast::aggregate_view_info &info : desc.view_infos()) {
    this->generate(info);
  }

  if (desc.size()) {

    out_ << "struct " << export_symbol_uppercase_ << " templates_description\n"
         << "  : mfast::templates_description\n"
         << "{\n"
         << "  typedef std::tuple<";

    bool first = true;
    for (std::size_t i = 0; i < desc.size(); ++i) {
      if (desc[i]->id() > 0) {
        if (!first) {
          out_ << ",\n"
               << "                       ";
        } else
          first = false;
        out_ << cpp_name(desc[i]->name());
      }
    }

    out_ << "> types;\n"
         << "  templates_description();\n"
         << "  static const templates_description* instance();\n"
         << "};\n\n";

    out_ << "inline const templates_description* description()\n"
         << "{\n"
         << "  return templates_description::instance();\n"
         << "}\n\n";
  }

  out_ << "#include \"" << filebase_ << inl_fileext_ << "\"\n"
       << "}\n";
  for (auto it = outer_ns_.begin(); it != outer_ns_.end(); ++it)
    out_ << "}\n";
}

void hpp_gen::visit(const mfast::enum_field_instruction *inst, void *pIndex) {
  std::string name(cpp_name(inst));

  if (inst->ref_instruction()) {
    std::string actual_type_name = cpp_type_of(inst, &dependency_);
    header_cref_ << indent << "typedef " << actual_type_name << "_cref " << name
                 << "_cref;\n";
    header_mref_ << indent << "typedef " << actual_type_name << "_mref " << name
                 << "_mref;\n";
  } else {
    // this is the enum definition
    header_cref_ << indent << "struct ";

    header_cref_ << export_symbol_uppercase_ << name << "\n" << indent << "{\n"
                 << indent << "  enum element {";

    for (uint64_t i = 0; i < inst->num_elements(); ++i) {
      if (i != 0)
        header_cref_ << ",";
      header_cref_ << "\n" << indent << "    " << cpp_name(inst->elements()[i]);
      if (inst->element_values()) {
        header_cref_ << " = " << inst->element_values()[i];
      }
    }

    header_cref_ << "\n" << indent << "  };\n" << indent
                 << "  typedef mfast::enum_field_instruction_ex<" << name
                 << "> instruction_type;\n" << indent
                 << "  static const instruction_type* instruction();\n"
                 << indent << "};\n\n";

    header_cref_ << indent << "class " << name << "_cref\n" << indent
                 << "  : public mfast::enum_cref_ex<" << name << "_cref, "
                 << name << ">\n" << indent << "{\n" << indent << "  public:\n"
                 << indent << "    typedef mfast::enum_cref_ex<" << name
                 << "_cref, " << name << "> base_type;\n" << indent
                 << "    typedef " << name << "::element element_type;\n"
                 << indent << "    typedef " << name
                 << "::instruction_type instruction_type;\n" << indent << "    "
                 << name << "_cref(\n" << indent
                 << "      const mfast::value_storage* storage=nullptr,\n" << indent
                 << "      instruction_cptr            instruction=nullptr);\n\n"
                 << indent << "    explicit " << name
                 << "_cref(const field_cref& other);\n\n" << indent
                 << "    element_type value() const;\n\n";

    for (uint64_t i = 0; i < inst->num_elements_; ++i) {
      std::string element_name = cpp_name(inst->elements_[i]);
      header_cref_ << indent << "    bool is_" << element_name << "() const;\n";
    }

    header_cref_ << indent << "};\n\n";

    header_mref_ << indent << "class " << name << "_mref\n" << indent
                 << "  : public mfast::enum_mref_ex<" << name << "_mref, "
                 << name << "_cref>\n" << indent << "{\n" << indent
                 << "  public:\n" << indent
                 << "    typedef  mfast::enum_mref_ex<" << name << "_mref, "
                 << name << "_cref> base_type;\n" << indent << "    typedef "
                 << name << "::element element_type;\n" << indent << "    "
                 << name << "_mref(\n" << indent
                 << "      mfast::allocator*     alloc=nullptr,\n" << indent
                 << "      mfast::value_storage* storage=nullptr,\n" << indent
                 << "      instruction_cptr      instruction=nullptr);\n"

                 << indent << "    explicit " << name
                 << "_mref(const mfast::field_mref_base& other);\n\n";

    for (uint64_t i = 0; i < inst->num_elements_; ++i) {
      std::string element_name = cpp_name(inst->elements_[i]);
      header_mref_ << indent << "    void as_" << element_name << "() const;\n";
    }

    header_mref_ << indent << "};\n\n";
  }

  if (pIndex) {
    header_cref_ << indent << name << "_cref get_" << name << "() const;\n";
    header_cref_ << indent << name << "_cref try_get_" << name << "() const;\n";
    if (inst->field_operator() != mfast::operator_constant)
      header_mref_ << indent << name << "_mref set_" << name << "() const;\n";
    if (inst->optional()) {
      header_mref_ << indent << "void "
                   << " omit_" << name << "() const;\n";
    }
  } else {
    content_ << header_cref_.str() << header_mref_.str();

    header_cref_.clear();
    header_cref_.str("");
    header_mref_.clear();
    header_mref_.str("");
  }
}

void hpp_gen::generate(const mfast::aggregate_view_info &info) {
  std::string ns_prefix;
  std::string my_name = cpp_name(info.name_);

  if (this->cpp_ns_ != info.instruction_->cpp_ns()) {
    ns_prefix = info.instruction_->cpp_ns();
    ns_prefix += "::";
  }

  out_ << "  class " << export_symbol_uppercase_ << my_name << "\n"
       << "  {\n"
       << "  public:\n"
       << "    typedef mfast::view_iterator iterator;\n"
       << "    typedef mfast::view_iterator const_iterator;\n"
       << "    " << my_name << "(const " << ns_prefix
       << cpp_name(info.instruction_->name()) << "_cref& ref);\n\n"
       << "    iterator begin() const;\n"
       << "    iterator end() const;\n"
       << "    template <typename FieldAccessor>\n"
       << "    void accept_accessor(FieldAccessor& accessor) const;\n\n"
       << "  private:\n"
       << "    " << ns_prefix << cpp_name(info.instruction_->name())
       << "_cref ref_;\n"
       << "    static const mfast::aggregate_view_info info_;\n"
       << "  };\n\n";
}
