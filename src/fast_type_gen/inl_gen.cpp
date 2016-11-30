// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "inl_gen.h"

const char *get_operator_tag(const mfast::field_instruction *inst) {
  const char *names[] = {
      "none_operator_tag",    "constant_operator_tag", "delta_operator_tag",
      "default_operator_tag", "copy_operator_tag",     "increment_operator_tag",
      "tail_operator_tag",
  };
  return names[inst->field_operator()];
}

// properties() is not virtual function, we need to overload it
template <typename Instruction>
std::string get_properties_type(const Instruction *inst) {
  std::stringstream strm;
  strm << "properties_type< " << inst->properties() << "> ";
  return strm.str();
}

using namespace mfast;
struct ext_cref_type_getter : mfast::field_instruction_visitor {
  std::stringstream out_;

public:
  ext_cref_type_getter() {}

  std::string get() { return out_.str(); }

  virtual void visit(const int32_field_instruction *inst, void *) override {
    out_ << "ext_cref<int32_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint32_field_instruction *inst, void *) override {
    out_ << "ext_cref<uint32_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const int64_field_instruction *inst, void *) override {
    out_ << "ext_cref<int64_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint64_field_instruction *inst, void *) override {
    out_ << "ext_cref<uint64_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const decimal_field_instruction *inst, void *) override {
    out_ << "ext_cref<decimal_cref, ";
    if (inst->field_type() == field_type_exponent) {
      out_ << "std::tuple<" << get_operator_tag(inst) << ", "
           << get_operator_tag(inst->mantissa_instruction()) << ">, "
           << "std::tuple<" << get_properties_type(inst) << ", "
           << get_properties_type(inst->mantissa_instruction()) << "> >";
    } else {
      out_ << get_operator_tag(inst) << ", " << get_properties_type(inst)
           << ">";
    }
  }

  virtual void visit(const ascii_field_instruction *inst, void *) override {
    out_ << "ext_cref<ascii_string_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const unicode_field_instruction *inst, void *) override {
    out_ << "ext_cref<unicode_string_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const byte_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_cref<byte_vector_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const group_field_instruction *inst, void *) override {
    out_ << "ext_cref< " << codegen_base::cpp_name(inst)
         << "_cref,  group_type_tag, " << get_properties_type(inst) << ">";
  }

  virtual void visit(const sequence_field_instruction *inst, void *) override;

  virtual void visit(const template_instruction *inst, void *) override {
    out_ << "ext_cref< " << codegen_base::cpp_name(inst)
         << "_cref, group_type_tag, " << get_properties_type(inst) << ">";
  }

  virtual void visit(const templateref_instruction *, void *) override {
    out_ << "nested_message_cref";
  }

  virtual void visit(const int32_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_cref< int32_vector_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint32_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_cref< uint32_vector_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const int64_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_cref< int64_vector_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint64_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_cref< uint64_vector_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const enum_field_instruction *inst, void *) override {
    out_ << "ext_cref<enum_cref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }
};

std::string get_ext_cref_type(const field_instruction *inst) {
  ext_cref_type_getter getter;
  inst->accept(getter, nullptr);
  return getter.get();
}

void ext_cref_type_getter::visit(const sequence_field_instruction *inst,
                                 void *) {
  const uint32_field_instruction *length_inst = inst->length_instruction();
  const field_instruction *element_inst =
      codegen_base::get_element_instruction(inst);

  out_ << "ext_cref< sequence_cref, ext_cref<uint32_cref, "
       << get_operator_tag(length_inst) << ", "
       << get_properties_type(length_inst) << ">, ";
  if (element_inst)
    out_ << "ext_cref< typename " << codegen_base::cpp_name(inst)
         << "_cref::reference, " << get_operator_tag(element_inst) << ", "
         << get_properties_type(element_inst) << " > >";
  else
    out_ << "ext_cref< typename " << codegen_base::cpp_name(inst)
         << "_cref::reference, sequence_element_tag,"
         << get_properties_type(inst) << "> >";
}

struct ext_mref_type_getter : mfast::field_instruction_visitor {
  std::stringstream out_;

public:
  ext_mref_type_getter() {}

  std::string get() { return out_.str(); }

  virtual void visit(const int32_field_instruction *inst, void *) override {
    out_ << "ext_mref<int32_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint32_field_instruction *inst, void *) override {
    out_ << "ext_mref<uint32_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const int64_field_instruction *inst, void *) override {
    out_ << "ext_mref<int64_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint64_field_instruction *inst, void *) override {
    out_ << "ext_mref<uint64_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const decimal_field_instruction *inst, void *) override {
    out_ << "ext_mref<decimal_mref, ";
    if (inst->field_type() == field_type_exponent) {
      out_ << "std::tuple<" << get_operator_tag(inst) << ", "
           << get_operator_tag(inst->mantissa_instruction()) << ">, "
           << "std::tuple<" << get_properties_type(inst) << ", "
           << get_properties_type(inst->mantissa_instruction()) << "> >";
    } else {
      out_ << get_operator_tag(inst) << ", " << get_properties_type(inst)
           << ">";
    }
  }

  virtual void visit(const ascii_field_instruction *inst, void *) override {
    out_ << "ext_mref<ascii_string_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const unicode_field_instruction *inst, void *) override {
    out_ << "ext_mref<unicode_string_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const byte_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_mref<byte_vector_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const group_field_instruction *inst, void *) override {
    out_ << "ext_mref< " << codegen_base::cpp_name(inst)
         << "_mref, group_type_tag, " << get_properties_type(inst) << ">";
  }

  virtual void visit(const sequence_field_instruction *inst, void *) override;
  virtual void visit(const template_instruction *inst, void *) override {
    out_ << "ext_mref< " << codegen_base::cpp_name(inst)
         << "_mref, group_type_tag, " << get_properties_type(inst) << ">";
  }

  virtual void visit(const templateref_instruction *, void *) override {
    out_ << "nested_message_mref";
  }

  virtual void visit(const int32_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_mref<int32_vector_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint32_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_mref<uint32_vector_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const int64_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_mref<int64_vector_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const uint64_vector_field_instruction *inst,
                     void *) override {
    out_ << "ext_mref<uint64_vector_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }

  virtual void visit(const enum_field_instruction *inst, void *) override {
    out_ << "ext_mref<enum_mref, " << get_operator_tag(inst) << ", "
         << get_properties_type(inst) << ">";
  }
};

std::string get_ext_mref_type(const field_instruction *inst) {
  ext_mref_type_getter getter;
  inst->accept(getter, nullptr);
  return getter.get();
}

void ext_mref_type_getter::visit(const sequence_field_instruction *inst,
                                 void *) {
  const uint32_field_instruction *length_inst = inst->length_instruction();
  const field_instruction *element_inst =
      codegen_base::get_element_instruction(inst);

  out_ << "ext_mref< sequence_mref, ext_mref<uint32_mref, "
       << get_operator_tag(length_inst) << ", "
       << get_properties_type(length_inst) << ">, ";
  if (element_inst && element_inst != inst)
    out_ << "ext_mref< typename " << codegen_base::cpp_name(inst)
         << "_mref::reference, " << get_operator_tag(element_inst) << ", "
         << get_properties_type(element_inst) << " > >";
  else
    out_ << "ext_mref< typename " << codegen_base::cpp_name(inst)
         << "_mref::reference, sequence_element_tag,"
         << get_properties_type(inst) << "> >";
}

void inl_gen::traverse(const mfast::group_field_instruction *inst,
                       const char *name_suffix) {
  std::string saved_mref_scope = mref_scope_.str();
  mref_scope_ << cpp_name(inst) << name_suffix << "_mref::";
  codegen_base::traverse(inst, name_suffix);
  reset_scope(mref_scope_, saved_mref_scope);
}

void inl_gen::gen_primitive(const char *cpp_type,
                            const mfast::field_instruction *inst,
                            void *pIndex) {
  std::string name(cpp_name(inst));
  std::stringstream cref_type_name;
  cref_type_name << "mfast::" << cpp_type << "_cref";
  std::stringstream mref_type_name;
  mref_type_name << "mfast::" << cpp_type << "_mref";

  gen_accessors(inst, name, cref_type_name.str(), mref_type_name.str(), pIndex);
}

void inl_gen::visit(const mfast::int32_field_instruction *inst, void *pIndex) {
  gen_primitive("int32", inst, pIndex);
}

void inl_gen::visit(const mfast::uint32_field_instruction *inst, void *pIndex) {
  gen_primitive("uint32", inst, pIndex);
}

void inl_gen::visit(const mfast::int64_field_instruction *inst, void *pIndex) {
  gen_primitive("int64", inst, pIndex);
}

void inl_gen::visit(const mfast::uint64_field_instruction *inst, void *pIndex) {
  gen_primitive("uint64", inst, pIndex);
}

void inl_gen::visit(const mfast::decimal_field_instruction *inst,
                    void *pIndex) {
  gen_primitive("decimal", inst, pIndex);
}

void inl_gen::visit(const mfast::ascii_field_instruction *inst, void *pIndex) {
  gen_primitive("ascii_string", inst, pIndex);
}

void inl_gen::visit(const mfast::unicode_field_instruction *inst,
                    void *pIndex) {
  gen_primitive("unicode_string", inst, pIndex);
}

void inl_gen::visit(const mfast::byte_vector_field_instruction *inst,
                    void *pIndex) {
  gen_primitive("byte_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::int32_vector_field_instruction *inst,
                    void *pIndex) {
  gen_primitive("int32_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::uint32_vector_field_instruction *inst,
                    void *pIndex) {
  gen_primitive("uint32_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::int64_vector_field_instruction *inst,
                    void *pIndex) {
  gen_primitive("int64_vector", inst, pIndex);
}

void inl_gen::visit(const mfast::uint64_vector_field_instruction *inst,
                    void *pIndex) {
  gen_primitive("uint64_vector", inst, pIndex);
}

template <typename T> struct visitor_print_proxy {
  T *inst_;
  visitor_print_proxy(T *inst) : inst_(inst) {}
};

template <typename T> inline visitor_print_proxy<T> print_visitor(T *inst) {
  return visitor_print_proxy<T>(inst);
}

template <typename T>
std::ostream &operator<<(std::ostream &os, visitor_print_proxy<T> p) {
  if (p.inst_->subinstructions().size()) {
    os << "visitor";
  }
  return os;
}

void inl_gen::visit(const mfast::group_field_instruction *inst, void *pIndex) {
  std::string name(cpp_name(inst));

  std::size_t index =
      pIndex == nullptr ? 0 : *static_cast<std::size_t *>(pIndex);

  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";

  bool embed_only_dyn_tempateref = contains_only_templateref(inst);

  std::stringstream cref_strm;
  std::stringstream mref_strm;

  if (embed_only_dyn_tempateref) {
    cref_strm << "mfast::aggregate_cref((*this)[" << index << "])[0]";
    mref_strm << "mfast::aggregate_mref((*this)[" << index << "])[0]";
  } else {
    cref_strm << "(*this)[" << index << "]";
    mref_strm << "(*this)[" << index << "]";
  }

  if (pIndex) {
    out_ << "\ninline " << cref_type_name << "\n" << cref_scope_.str() << "get_"
         << name << "() const\n"
         << "{\n";
    if (inst->optional())
      out_ << "  if ("
           << "(*this)[" << index << "].absent())\n"
           << "    return " << cref_type_name << "(nullptr, " << cref_type_name
           << "::instruction_cptr(nullptr));\n";
    out_ << "  return static_cast<" << cref_type_name << ">(" << cref_strm.str()
         << ");\n"
         << "}\n\n"
         << "\ninline " << cref_type_name << "\n" << cref_scope_.str()
         << "try_get_" << name << "() const\n"
         << "{\n";
    if (inst->optional())
      out_ << "  if ("
           << "(*this)[" << index << "].absent())\n"
           << "    throw mfast::bad_optional_access();\n";
    out_ << "  return static_cast<" << cref_type_name << ">(" << cref_strm.str()
         << ");\n"
         << "}\n\n"
         << "inline " << mref_type_name << "\n" << mref_scope_.str() << "set_"
         << name << "() const\n"
         << "{\n";
    if (inst->optional())
      out_ << "  this->field_storage(" << index << ")->present(true);\n";
    out_ << "  return " << mref_type_name << "(" << mref_strm.str() << ");\n"
         << "}\n\n";

    if (inst->optional()) {
      out_ << "inline\n"
           << "void\n" << mref_scope_.str() << "omit_" << name << "() const\n"
           << "{\n"
           << "  (*this)[" << index << "].omit();\n"
           << "}\n\n";
    }

    if (inst->ref_instruction()) {
      out_ << "inline void\n" << mref_scope_.str() << "link_" << name
           << "(const " << mref_type_name << "& ref) const\n"
           << "{\n"
           << "  this->link_group_at(" << index << ", ref);\n"
           << "}\n\n"
           << "inline void\n" << mref_scope_.str() << "unlink_" << name
           << "() const\n"
           << "{\n"
           << "  this->unlink_group_at(" << index << ");\n"
           << "}\n\n";
    }
  }

  if (inst->ref_instruction() == nullptr && !embed_only_dyn_tempateref) {

    out_ << "inline\n" << cref_type_name << "::" << name << "_cref()\n"
         << "  : base_type(nullptr, nullptr)\n"
         << "{\n"
         << "}\n\n"
         << "template <typename T>"
         << "inline\n" << cref_type_name << "::" << name << "_cref(\n"
         << "  typename std::enable_if<std::is_same<typename T::cref_type, "
         << name
         << "_cref>::value, const mfast::value_storage*>::type storage_array,\n"
         << "  const T* instruction)\n"
         << "  : base_type(storage_array, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << cref_type_name << "::" << name << "_cref(\n"
         << "  const mfast::field_cref& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n";

    out_ << "template <typename Visitor>\n"
         << "inline void " << cref_type_name << "::accept(Visitor& "
         << print_visitor(inst) << ")\n"
         << "{\n"
         << "  using namespace mfast;\n";

    for (std::size_t i = 0; i < inst->subinstructions().size(); ++i) {
      const field_instruction *subinst = inst->subinstructions()[i];
      ;
      out_ << "  visitor.visit(" << get_ext_cref_type(subinst) << " ((*this)["
           << i << "]) );\n";
    }

    out_ << "}\n\n";

    out_ << "inline\n" << mref_type_name << "::" << name << "_mref()\n"
         << "  : base_type(nullptr, nullptr, nullptr)\n"
         << "{\n"
         << "}\n\n"
         << "template <typename T>"
         << "inline\n" << mref_type_name << "::" << name << "_mref(\n"
         << "  mfast::allocator* alloc,\n"
         << "  typename std::enable_if<std::is_same<typename T::cref_type, "
         << name
         << "_cref>::value, mfast::value_storage*>::type storage_array,\n"
         << "  const T*          instruction)\n"
         << "  : base_type(alloc, storage_array, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << mref_type_name << "::" << name << "_mref(\n"
         << "  const mfast::field_mref_base& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n";

    out_ << "template <typename Visitor>\n"
         << "inline void " << mref_type_name << "::accept(Visitor& "
         << print_visitor(inst) << ")\n"
         << "{\n"
         << "  using namespace mfast;\n";

    for (std::size_t i = 0; i < inst->subinstructions().size(); ++i) {
      const field_instruction *subinst = inst->subinstructions()[i];
      out_ << "  visitor.visit(" << get_ext_mref_type(subinst) << " ((*this)["
           << i << "]) );\n";
    }

    out_ << "}\n\n";

    if (!pIndex) {
      out_
          << "inline\n" << name << "::" << name << "(\n"
          << "  mfast::allocator* alloc)\n"
          << "  : base_type(alloc, instruction(), this->data())\n"
          << "{\n"
          << "}\n\n"
          << "inline\n" << name << "::" << name << "(\n"
          << "  const " << name << "_cref& other,\n"
          << "  mfast::allocator* alloc)\n"
          << "  : base_type(alloc, instruction(), this->data(), "
             "other.field_storage(0))\n"
          << "{\n"
          << "}\n\n"
          << "inline "
          << name
          << "::cref_type\n"
          << name
          << "::ref() const\n"
          << "{\n"
          << "  return "
          << name
          << "::cref_type(my_storage_.of_group.content_, instruction());\n"
          << "}\n\n"
          << "inline "
          << name
          << "::cref_type\n"
          << name
          << "::cref() const\n"
          << "{\n"
          << "  return "
          << name
          << "::cref_type(my_storage_.of_group.content_, instruction());\n"
          << "}\n\n"
          << "inline "
          << name
          << "::mref_type\n"
          << name
          << "::ref()\n"
          << "{\n"
          << "  return "
          << name
          << "::mref_type(alloc_, my_storage_.of_group.content_, "
             "instruction());\n"
          << "}\n\n"
          << "inline "
          << name
          << "::mref_type\n"
          << name
          << "::mref()\n"
          << "{\n"
          << "  return "
          << name
          << "::mref_type(alloc_, my_storage_.of_group.content_, "
             "instruction());\n"
          << "}\n\n"
          << "inline\n"
          << name
          << "::"
          << name
          << "("
          << name
          << "&& other) BOOST_NOEXCEPT\n"
          << "  : base_array(static_cast<const base_array&>(other))\n"
          << "  , base_type(std::move(other), this->data())\n"
          << "{\n"
          << "}\n\n"
          << "inline\n"
          << name
          << "::"
          << name
          << "(const "
          << name
          << "& other) \n"
          << "  : base_type(other.allocator(), other.instruction(), "
             "this->data(), other.data())\n"
          << "{\n"
          << "}\n\n"
          << "inline "
          << name
          << "&\n"
          << name
          << "::operator =("
          << name
          << "&& other) BOOST_NOEXCEPT\n"
          << "{\n"
          << "  base_type::assign(std::move(other), this->data());\n"
          << "  base_array::operator=(static_cast<const base_array&>(other));\n"
          << "  return *this;\n"
          << "}\n\n"
          << "inline "
          << name
          << "&\n"
          << name
          << "::operator =(const "
          << name
          << "& other)\n"
          << "{\n"
          << "  "
          << name
          << " tmp(other);\n"
          << "  return this->operator=(std::move(tmp));\n"
          << "}\n\n";
    }

    traverse(inst, "");
  }
}

void inl_gen::visit(const mfast::sequence_field_instruction *inst,
                    void *pIndex) {
  std::string name(cpp_name(inst));

  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";

  gen_accessors(inst, name, cref_type_name, mref_type_name, pIndex);

  const mfast::field_instruction *element_instruction =
      get_element_instruction(inst);

  if (inst->ref_instruction() == nullptr && element_instruction == nullptr) {
    out_ << "inline\n" << cref_scope_.str() << name << "_element_cref::" << name
         << "_element_cref(\n"
         << "  const mfast::value_storage*   storage,\n"
         << "  " << cref_scope_.str() << name
         << "_element_cref::instruction_cptr instruction)\n"
         << "  : base_type(storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << mref_scope_.str() << name << "_element_mref::" << name
         << "_element_mref(\n"
         << "  mfast::allocator*               alloc,\n"
         << "  mfast::value_storage*         storage,\n"
         << "  " << mref_scope_.str() << name
         << "_element_mref::instruction_cptr instruction)\n"
         << "  : base_type(alloc,storage, instruction)\n"
         << "{\n"
         << "}\n\n";

    out_ << "template <typename Visitor>\n"
         << "inline void " << cref_scope_.str() << name
         << "_element_cref::accept(Visitor& " << print_visitor(inst) << ")\n"
         << "{\n"
         << "  using namespace mfast;\n";

    for (std::size_t i = 0; i < inst->subinstructions().size(); ++i) {
      const field_instruction *subinst = inst->subinstructions()[i];
      ;
      out_ << "  visitor.visit(" << get_ext_cref_type(subinst) << " ((*this)["
           << i << "]) );\n";
    }

    out_ << "}\n\n";

    out_ << "template <typename Visitor>\n"
         << "inline void " << mref_scope_.str() << name
         << "_element_mref::accept(Visitor& " << print_visitor(inst) << ")\n"
         << "{\n"
         << "  using namespace mfast;\n";

    for (std::size_t i = 0; i < inst->subinstructions().size(); ++i) {
      const field_instruction *subinst = inst->subinstructions()[i];
      ;
      out_ << "  visitor.visit(" << get_ext_mref_type(subinst) << " ((*this)["
           << i << "]) );\n";
    }

    out_ << "}\n\n";

    traverse(inst, "_element");
  }

  if (pIndex == nullptr) {
    out_ << "inline\n" << name << "::" << name << "(\n"
         << "  mfast::allocator* alloc)\n"
         << "  : base_type(alloc, instruction(), nullptr)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << name << "::" << name << "(\n"
         << "  const " << name << "_cref& other,\n"
         << "  mfast::allocator* alloc)\n"
         << "  : base_type(reinterpret_cast<const "
            "mfast::sequence_cref&>(other), alloc)\n"
         << "{\n"
         << "}\n\n"
         << "inline "
         << name
         << "::cref_type\n"
         << name
         << "::ref() const\n"
         << "{\n"
         << "  return "
         << name
         << "::cref_type(&my_storage_, instruction());\n"
         << "}\n\n"
         << "inline "
         << name
         << "::cref_type\n"
         << name
         << "::cref() const\n"
         << "{\n"
         << "  return "
         << name
         << "::cref_type(&my_storage_, instruction());\n"
         << "}\n\n"
         << "inline "
         << name
         << "::mref_type\n"
         << name
         << "::ref()\n"
         << "{\n"
         << "  return "
         << name
         << "::mref_type(alloc_, &my_storage_, instruction());\n"
         << "}\n\n"
         << "inline "
         << name
         << "::mref_type\n"
         << name
         << "::mref()\n"
         << "{\n"
         << "  return "
         << name
         << "::mref_type(alloc_, &my_storage_, instruction());\n"
         << "}\n\n";
  }
}

void inl_gen::visit(const mfast::template_instruction *inst, void *) {
  // if (inst->subinstructions().size() == 0)
  //   return;

  std::string name(cpp_name(inst));

  out_ << "inline\n" << name << "_cref::" << name << "_cref()\n"
       << "  : base_type(nullptr, nullptr)\n"
       << "{\n"
       << "}\n\n"
       << "template <typename T>"
       << "inline\n" << name << "_cref::" << name << "_cref(\n"
       << "  typename std::enable_if<std::is_same<typename T::cref_type, "
       << name
       << "_cref>::value, const mfast::value_storage*>::type storage_array,\n"
       << "  const T* instruction)\n"
       << "  : base_type(storage_array, instruction)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n" << name << "_cref::" << name << "_cref(\n"
       << "  const mfast::message_cref& other)\n"
       << "  : base_type(other)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n" << name << "_cref::" << name << "_cref(\n"
       << "  const mfast::field_cref& other)\n"
       << "  : "
          "base_type(mfast::field_cref_core_access::storage_of(other)->of_"
          "group.content_,\n"
       << "              static_cast<instruction_cptr>(other.instruction()))\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name
       << "_cref::operator mfast::message_cref()\n"
       << "{\n"
       << "  return mfast::message_cref(this->storage(), "
          "static_cast<instruction_cptr>(this->instruction()));\n"
       << "}\n\n";

  out_ << "template <typename Visitor>\n"
       << "inline void " << name << "_cref::accept(Visitor& "
       << print_visitor(inst) << ")\n"
       << "{\n"
       << "  using namespace mfast;\n";

  for (std::size_t i = 0; i < inst->subinstructions().size(); ++i) {
    const field_instruction *subinst = inst->subinstructions()[i];
    ;
    out_ << "  visitor.visit(" << get_ext_cref_type(subinst) << " ((*this)["
         << i << "]) );\n";
  }

  out_ << "}\n\n";

  out_ << "inline\n" << name << "_mref::" << name << "_mref()\n"
       << "  : base_type(nullptr, nullptr, nullptr)\n"
       << "{\n"
       << "}\n\n"
       << "template <typename T>"
       << "inline\n" << name << "_mref::" << name << "_mref(\n"
       << "  mfast::allocator* alloc,\n"
       << "  typename std::enable_if<std::is_same<typename T::cref_type, "
       << name << "_cref>::value, mfast::value_storage*>::type storage_array,\n"
       << "  const T*          instruction)\n"
       << "  : base_type(alloc, storage_array, instruction)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n" << name << "_mref::" << name << "_mref(\n"
       << "  const mfast::message_mref& other)\n"
       << "  : base_type(other)\n"
       << "{\n"
       << "}\n\n"
       << "inline\n" << name << "_mref::" << name << "_mref(\n"
       << "  const mfast::field_mref_base& other)\n"
       << "  : base_type(other.allocator(),\n"
       << "              "
          "mfast::field_mref_core_access::storage_of(other)->of_group.content_,"
          "\n"
       << "              static_cast<instruction_cptr>(other.instruction()))\n"
       << "{\n"
       << "}\n\n"
       << "inline\n"
       << name
       << "_mref::operator mfast::message_mref()\n"
       << "{\n"
       << "  return mfast::message_mref(this->allocator(), "
          "const_cast<mfast::value_storage*>(this->storage()), "
          "static_cast<instruction_cptr>(this->instruction()));\n"
       << "}\n\n";

  out_ << "template <typename Visitor>\n"
       << "inline void " << name << "_mref::accept(Visitor& "
       << print_visitor(inst) << ")\n"
       << "{\n"
       << "  using namespace mfast;\n";

  for (std::size_t i = 0; i < inst->subinstructions().size(); ++i) {
    const field_instruction *subinst = inst->subinstructions()[i];
    ;
    out_ << "  visitor.visit(" << get_ext_mref_type(subinst) << " ((*this)["
         << i << "]) );\n";
  }

  out_ << "}\n\n";

  out_ << "inline\n" << name << "::" << name << "(\n"
       << "  mfast::allocator* alloc)\n"
       << "  : base_type(alloc, instruction(), this->data())\n"
       << "{\n"
       << "}\n\n"
       << "inline\n" << name << "::" << name << "(\n"
       << "  const " << name << "_cref& other,\n"
       << "  mfast::allocator* alloc)\n"
       << "  : base_type(alloc, instruction(), this->data(), "
          "other.field_storage(0))\n"
       << "{\n"
       << "}\n\n"
       << "#ifdef MFAST_JSON_H\n"
       << name
       << "::"
       << name
       << "(\n"
       << "  const char* json_initializer,\n"
       << "  int json_object_tag_mask,\n"
       << "  mfast::allocator* alloc)\n"
       << "  : base_type(alloc, instruction(), this->data())\n"
       << "{\n"
       << "  std::stringstream strm(json_initializer);\n"
       << "  mfast::json::decode(strm, this->mref(), json_object_tag_mask);\n"
       << "}\n\n"
       << "#endif\n"
       << "inline "
       << name
       << "::cref_type\n"
       << name
       << "::ref() const\n"
       << "{\n"
       << "  return "
       << name
       << "::cref_type(my_storage_.of_group.content_, instruction());\n"
       << "}\n\n"
       << "inline "
       << name
       << "::cref_type\n"
       << name
       << "::cref() const\n"
       << "{\n"
       << "  return "
       << name
       << "::cref_type(my_storage_.of_group.content_, instruction());\n"
       << "}\n\n"
       << "inline "
       << name
       << "::mref_type\n"
       << name
       << "::ref()\n"
       << "{\n"
       << "  return "
       << name
       << "::mref_type(alloc_, my_storage_.of_group.content_, instruction());\n"
       << "}\n\n"
       << "inline "
       << name
       << "::mref_type\n"
       << name
       << "::mref()\n"
       << "{\n"
       << "  return "
       << name
       << "::mref_type(alloc_, my_storage_.of_group.content_, instruction());\n"
       << "}\n\n";

  this->traverse(inst, "");
}

void inl_gen::visit(const mfast::templateref_instruction *, void *pIndex) {
  std::size_t index = *static_cast<std::size_t *>(pIndex);

  out_ << "\n"
       << "inline\n"
       << "mfast::nested_message_cref\n" << cref_scope_.str()
       << "get_nested_message" << index << "() const\n"
       << "{\n"
       << "  return mfast::nested_message_cref((*this)[" << index << "]);\n"
       << "}\n\n" << cref_scope_.str() << "try_get_nested_message" << index
       << "() const\n"
       << "{\n"
       << "  return mfast::nested_message_cref((*this)[" << index << "]);\n"
       << "}\n\n"
       << "inline\n"
       << "mfast::nested_message_mref\n" << mref_scope_.str()
       << "set_nested_message" << index << "() const\n"
       << "{\n"
       << "  return mfast::nested_message_mref((*this)[" << index << "]);\n"
       << "}\n\n";
}

void inl_gen::generate(mfast::dynamic_templates_description &desc) {

  generate_comment();

  codegen_base::traverse(desc);
  for (auto &&info : desc.view_infos()) {
    this->generate(info);
  }
}

void inl_gen::visit(const mfast::enum_field_instruction *inst, void *pIndex) {
  std::string name(cpp_name(inst));
  std::string cref_type_name = cref_scope_.str() + name + "_cref";
  std::string mref_type_name = mref_scope_.str() + name + "_mref";

  if (inst->ref_instruction() == nullptr) {

    out_ << "inline\n" << cref_type_name << "::" << name << "_cref(\n"
         << "  const mfast::value_storage*   storage,\n"
         << "  " << cref_type_name << "::instruction_cptr instruction)\n"
         << "  : base_type(storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << cref_type_name << "::" << name << "_cref(\n"
         << "  const mfast::field_cref& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << mref_type_name << "::" << name << "_mref(\n"
         << "  mfast::allocator*      alloc,\n"
         << "  mfast::value_storage*  storage,\n"
         << "  " << mref_type_name << "::instruction_cptr instruction)\n"
         << "  : base_type(alloc, storage, instruction)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << mref_type_name << "::" << name << "_mref(\n"
         << "  const mfast::field_mref_base& other)\n"
         << "  : base_type(other)\n"
         << "{\n"
         << "}\n\n"
         << "inline\n" << cref_type_name << "::element_type\n" << cref_type_name
         << "::value() const\n"
         << "{\n"
         << "  return static_cast<" << name
         << "::element>(base_type::value());\n"
         << "}\n\n";

    for (uint64_t i = 0; i < inst->num_elements_; ++i) {

      std::string element_name = cpp_name(inst->elements_[i]);

      out_ << "inline\n"
           << "bool " << cref_type_name << "::is_" << element_name
           << "() const\n"
           << "{\n"
           << "  return this->value() == " << name << "::" << element_name
           << ";\n"
           << "}\n\n"
           << "inline\n"
           << "void " << mref_type_name << "::as_" << element_name
           << "() const\n"
           << "{\n"
           << "  return this->as(" << name << "::" << element_name << ");\n"
           << "}\n\n";
    }
  }

  gen_accessors(inst, name, cref_type_name, mref_type_name, pIndex);
}

void inl_gen::gen_accessors(const mfast::field_instruction *inst,
                            const std::string &name,
                            const std::string &cref_type_name,
                            const std::string &mref_type_name, void *pIndex) {
  if (pIndex) {
    std::size_t index = *static_cast<std::size_t *>(pIndex);
    out_ << "inline " << cref_type_name << "\n" << cref_scope_.str() << "get_"
         << name << "() const\n"
         << "{\n"
         << "  return static_cast<" << cref_type_name << ">((*this)[" << index
         << "]);\n"
         << "}\n\n";

    out_ << "inline " << cref_type_name << "\n" << cref_scope_.str()
         << "try_get_" << name << "() const\n"
         << "{\n";

    if (inst->optional())
      out_ << "  if ("
           << "(*this)[" << index << "].absent())\n"
           << "    throw mfast::bad_optional_access();\n";

    out_ << "  return static_cast<" << cref_type_name << ">((*this)[" << index
         << "]);\n"
         << "}\n\n";

    if (!is_const_field(inst)) {
      out_ << "inline " << mref_type_name << "\n" << mref_scope_.str() << "set_"
           << name << "() const\n"
           << "{\n"
           << "  return static_cast<" << mref_type_name << ">((*this)[" << index
           << "]);\n"
           << "}\n\n";
    }
    if (inst->optional()) {
      out_ << "inline\n"
           << "void\n" << mref_scope_.str() << "omit_" << name << "() const\n"
           << "{\n"
           << "  (*this)[" << index << "].omit();\n"
           << "}\n\n";
    }
  }
}

void inl_gen::generate(const mfast::aggregate_view_info &info) {
  std::string ns_prefix;
  std::string my_name = cpp_name(info.name_);

  if (this->cpp_ns_ != info.instruction_->cpp_ns()) {
    ns_prefix = info.instruction_->cpp_ns();
    ns_prefix += "::";
  }

  out_ << "inline " << my_name << "::" << my_name << "(const " << ns_prefix
       << cpp_name(info.instruction_->name()) << "_cref& ref)\n"
       << "  : ref_(ref)\n"
       << "{\n"
       << "}\n\n"
       << "inline mfast::view_iterator " << my_name << "::begin() const\n"
       << "{\n"
       << "  return mfast::view_iterator(ref_, info_.data_.begin(), "
          "info_.max_depth_);\n"
       << "}\n\n"
       << "inline mfast::view_iterator  "
       << my_name
       << "::end() const\n"
       << "{\n"
       << "  return mfast::view_iterator(info_.data_.end()-1);\n"
       << "}\n\n"
       << "template <typename FieldAccessor>\n"
       << "inline void  "
       << my_name
       << "::accept_accessor(FieldAccessor& accessor) const\n"
       << "{\n"
       << "  for (auto f: *this)\n"
       << "  {\n"
       << "    if (f.present())"
       << "      mfast::apply_accessor(accessor, f);\n"
       << "  }\n"
       << "}\n";
}
