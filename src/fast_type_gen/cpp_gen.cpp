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
#include "cpp_gen.h"
#include <boost/io/ios_state.hpp>


std::string cpp_gen::prefix_string() const
{
  std::stringstream strm;
  for (std::size_t i = 0; i < prefixes_.size(); ++i) {
    strm << prefixes_[i] << "__";
  }
  return strm.str();
}

std::string cpp_gen::cref_scope() const
{
  std::stringstream strm;
  for (std::size_t i = 0; i < prefixes_.size(); ++i) {
    strm << prefixes_[i] << "_cref::";
  }
  return strm.str();
}

void cpp_gen::add_to_instruction_list(const std::string& name)
{
  std::stringstream strm;
  strm << "  &" << prefix_string() << name << "_instruction,\n";
  subinstructions_list_.back() += strm.str();
}

const char* get_operator_name(const mfast::field_instruction* inst)
{
  const char* names[] = {
    "operator_none",
    "operator_constant",
    "operator_delta",
    "operator_default",
    "operator_copy",
    "operator_increment",
    "operator_tail",
  };
  return names[inst->field_operator()];
}

const char* get_presence(const mfast::field_instruction* inst)
{
  if (inst->optional())
    return "presence_optional";
  return "presence_mandatory";
}

std::string
cpp_gen::gen_op_context(const char*                name,
                        const mfast::op_context_t* context)
{
  if (context == 0)
    return "0";

  out_ << "const static " << "op_context_t " << prefix_string() << name << "_opContext ={\n"
       << "  \"" << context->key_ << "\", \n"
       << "  \"" << context->ns_ << "\", \n"
       << "  \"" << context->dictionary_ << "\"};\n\n";

  std::stringstream result;
  result << "&" <<  prefix_string() << name << "_opContext";
  return result.str();
}

void cpp_gen::gen_field(const mfast::field_instruction* inst,
                        const std::string&              context,
                        const char*                     cpp_type)
{
  std::string name( cpp_name( inst ) );
  std::size_t index = inst->field_index();
  // std::string context = gen_op_context(inst->name(), inst->op_context());

  out_ << "const static " << cpp_type << "_field_instruction\n"
       << prefix_string() << name << "_instruction(\n"
       << "  " << index << ",\n"
       << "  " << get_operator_name(inst) << ",\n"
       << "  " << get_presence(inst) << ",\n"
       << "  " << inst->id() << ", // id\n"
       << "  \""<< inst->name() << "\", // name\n"
       << "  \""<< inst->ns() << "\", // ns\n"
       << "  "<< context << ",  // opContext\n";

  add_to_instruction_list(name);
}

template <typename T>
std::string
get_initial_value(const mfast::value_storage& storage, const char* suffix)
{
  if (!storage.is_empty()) {
    std::stringstream strm;
    strm << storage.get<T>() << suffix;
    return strm.str();
  }
  return std::string ();
}

void cpp_gen::gen_integer(const mfast::integer_field_instruction_base* inst,
                          const char*                                  cpp_type,
                          const std::string&                           initial_value)
{
  gen_field(inst, gen_op_context(inst->name(), inst->op_context()), cpp_type);
  out_ << "  int_value_storage<"<< cpp_type << "_t>(" << initial_value   << "), // initial_value\n"
       << "  " << inst->tag() << "); // tag\n\n";
}

void cpp_gen::visit(const mfast::int32_field_instruction* inst, void*)
{
  gen_integer(inst, "int32", get_initial_value<int32_t>(inst->initial_value(), "") );
}

void cpp_gen::visit(const mfast::uint32_field_instruction* inst, void*)
{
  gen_integer(inst, "uint32", get_initial_value<uint32_t>(inst->initial_value(), "U") );
}

void cpp_gen::visit(const mfast::int64_field_instruction* inst, void*)
{
  gen_integer(inst, "int64", get_initial_value<int64_t>(inst->initial_value(), "LL") );
}

void cpp_gen::visit(const mfast::uint64_field_instruction* inst, void*)
{
  gen_integer(inst, "uint64", get_initial_value<uint64_t>(inst->initial_value(), "ULL") );
}

void cpp_gen::visit(const mfast::decimal_field_instruction* inst, void*)
{
  std::string name( cpp_name( inst ) );
  if (inst->mantissa_instruction() ) {
    // it has a separate mantissa instruction
    std::string mantisa_name(name);
    mantisa_name += "_mantissa";
    std::string context = gen_op_context(mantisa_name.c_str(), inst->mantissa_instruction()->op_context());

    out_ << "static mantissa_field_instruction\n"
         << prefix_string() << mantisa_name << "_instruction(\n"
         << "  " << get_operator_name(inst->mantissa_instruction()) << ",\n"
         << "  "<< context << ",  // mantissa opContext\n"
         << "  int_value_storage<int64_t>("
         << get_initial_value<int64_t>(inst->mantissa_instruction()->initial_value(), "LL")
         << "));// mantissa inital value\n\n";
  }

  gen_field(inst, gen_op_context(inst->name(), inst->op_context()), "decimal");

  if ( inst->field_type() == mfast::field_type_exponent)  {
    if (inst->mantissa_instruction() )
      out_ << "  &" << prefix_string() << name << "_mantissa_instruction,\n";
    else
      out_ << "  0, // mantissa_instruction\n";
  }

  const mfast::value_storage& init_value = inst->initial_value();

  out_ << "  decimal_value_storage(";
  if (!init_value.is_empty())
    out_ <<  init_value.of_decimal.mantissa_ << "LL, " << static_cast<int> (init_value.of_decimal.exponent_);
  out_ << "), // initial_value\n"
       << "  " << inst->tag() << "); // tag\n\n";

}

void cpp_gen::gen_string(const mfast::ascii_field_instruction* inst,
                         const char*                           charset)
{
  gen_field(inst, gen_op_context(inst->name(), inst->op_context()), charset);

  if (inst->initial_value().is_empty()) {
    out_ << "  "<< "string_value_storage()";
  }
  else {
    out_ << "  "<< "string_value_storage(\""<< inst->initial_value().get<char*>()
         << "\"," <<  inst->initial_value().array_length() << ")";
  }
}

void cpp_gen::visit(const mfast::ascii_field_instruction* inst, void*)
{
  gen_string(inst, "ascii");
  out_ <<  ", // initial_value\n"
       << "  " << inst->tag() << "); // tag\n\n";
}

void cpp_gen::visit(const mfast::unicode_field_instruction* inst, void*)
{
  gen_string(inst, "unicode");
  if (inst->length_name()) {
    out_ << ", // initial value\n"
         << "  " << inst->length_id() << ", // length id\n"
         << "  \"" <<  inst->length_name() << "\", // length name\n"
         << "  \"" << inst->length_ns() << "\"); // length ns\n\n";
  }
  else {
    out_ << ", // initial_value\n"
         << "  " << inst->tag() << "); // tag\n\n";
  }
}

void cpp_gen::visit(const mfast::byte_vector_field_instruction* inst, void*)
{
  gen_field(inst, gen_op_context(inst->name(), inst->op_context()), "byte_vector");

  if (inst->initial_value().is_empty()) {
    out_ << "  "<< "byte_vector_value_storage()";
  }
  else {
    out_ << "  "<< "byte_vector_value_storage(\"";
    const char* val = static_cast<const char*>(inst->initial_value().of_array.content_);
    {
      boost::io::ios_flags_saver ifs( out_ );
      out_ << std::hex << std::setfill('0') << std::setw(2);

      for (std::size_t i = 0; i < inst->initial_value().array_length(); ++i)
      {
        out_ << "\\x" << (0xFF & (int) val[i]);
      }
    }
    out_<< "\"," <<  inst->initial_value().array_length() << ")";
  }


  if (inst->length_name()) {
    out_ << ", // initial value\n"
         << "  " << inst->length_id() << ", // length id\n"
         << "  \"" <<  inst->length_name() << "\", // length name\n"
         << "  \"" << inst->length_ns() << "\", // length ns\n"
         << "  " << inst->tag() << "); // tag\n\n";
  }
  else {
    out_ << ", // initial_value\n"
         << "  " << inst->tag() << "); // tag\n\n";
  }
}

void cpp_gen::gen_int_vector(const char* cpp_type, const mfast::vector_field_instruction_base* inst)
{
  std::string name( cpp_name( inst ) );
  std::size_t index = inst->field_index();
  // std::string context = gen_op_context(inst->name(), inst->op_context());

  out_ << "const static " << cpp_type << "_field_instruction\n"
       << prefix_string() << name << "_instruction(\n"
       << "  " << index << ",\n"
       << "  " << get_presence(inst) << ",\n"
       << "  " << inst->id() << ", // id\n"
       << "  \""<< inst->name() << "\", // name\n"
       << "  \""<< inst->ns() << "\", // ns\n"
       << "  " << inst->tag() << "); // tag\n\n";

  add_to_instruction_list(name);
}

void cpp_gen::visit(const mfast::int32_vector_field_instruction* inst, void*)
{
  gen_int_vector("int32_vector", inst);
}

void cpp_gen::visit(const mfast::uint32_vector_field_instruction* inst, void*)
{
  gen_int_vector("uint32_vector", inst);
}

void cpp_gen::visit(const mfast::int64_vector_field_instruction* inst, void*)
{
  gen_int_vector("int64_vector", inst);
}

void cpp_gen::visit(const mfast::uint64_vector_field_instruction* inst, void*)
{
  gen_int_vector("uint64_vector", inst);
}

void cpp_gen::output_subinstructions()
{
  assert(subinstructions_list_.size());
  std::string content = subinstructions_list_.back();

  content.resize(content.size()-1);

  out_ << "const static field_instruction* " << prefix_string() << "subinstructions[] = {\n"
       << content << "\n"
       << "};\n\n";
  subinstructions_list_.pop_back();
}

bool cpp_gen::need_generate_subinstructions(const mfast::group_field_instruction* inst)
{
  return inst->ref_instruction()==0 && !this->contains_only_templateref(inst);
}

std::string
cpp_gen::get_subinstructions(const mfast::group_field_instruction* inst)
{
  using namespace mfast;
  std::stringstream subinstruction_arg;

  if (need_generate_subinstructions(inst)) {

    output_subinstructions();

    subinstruction_arg << "  "<< prefix_string() << "subinstructions,\n"
                       << "  "<< inst->subinstructions_count() << ", // num_fields\n";
    prefixes_.pop_back();

  }
  else
  {
    prefixes_.pop_back();

    if (this->contains_only_templateref(inst))
      subinstruction_arg << "  "<< "mfast::templateref_instruction::default_instruction(),\n"
                         << "  1, // num_fields\n";
    else {
      std::string qualified_name = cpp_type_of(inst);

      subinstruction_arg << "  "<< qualified_name << "::instruction(),\n";
    }
  }
  return subinstruction_arg.str();

}

void cpp_gen::visit(const mfast::group_field_instruction* inst, void* top_level)
{
  std::string name( cpp_name( inst ) );
  std::size_t index = inst->field_index();

  std::string qualified_name = name;
  if (inst->ref_instruction())
  {
    qualified_name = cpp_type_of(inst);
  }

  if (top_level) {
    out_ << "const " << qualified_name << "::instruction_type*\n"
         << name << "::instruction()\n"
         << "{\n";
  }
  else
    add_to_instruction_list(name);

  prefixes_.push_back(name);

  bool to_gen_subinstructions = need_generate_subinstructions(inst);

  if (  to_gen_subinstructions )
  {

    subinstructions_list_.resize(subinstructions_list_.size()+1);
    traverse(inst, "");
  }

  std::string subinstruction_arg = get_subinstructions(inst);

  if (top_level) {
    out_ << "const static " << name << "::instruction_type\n"
         << "  the_instruction(\n";
  }
  else if (  to_gen_subinstructions )
  {
    out_ << "const static " << cref_scope() << name << "_cref::instruction_type\n"
         << prefix_string() << name << "_instruction(\n";
  }
  else {
    out_ << "const static mfast::group_instruction_ex<" << cref_scope() << name << "_cref>\n"
         << prefix_string() << name << "_instruction(\n";
  }

  out_ << "  "<<  index << ",\n"
       << "  " << get_presence(inst) << ",\n"
       << "  " << inst->id() << ", // id\n"
       << "  \"" << inst->name() << "\", // name\n"
       << "  \"" << inst->ns() << "\", // ns\n"
       << "  \"" << inst->dictionary() << "\", // dictionary\n"
       << subinstruction_arg
       << "  \"" << inst->typeref_name() << "\", // typeRef name \n"
       << "  \"" << inst->typeref_ns() << "\", // typeRef ns \n"
       << "  \"\", // cpp_ns\n"
       << "  " << inst->tag() << "); // tag\n\n";

  if (top_level) {
    out_ << "  return &the_instruction;\n"
         << "}\n\n";
  }
}

void cpp_gen::visit(const mfast::sequence_field_instruction* inst, void* top_level)
{
  std::string name( cpp_name( inst ) );
  std::size_t index = inst->field_index();
  bool to_gen_subinstruction =  inst->ref_instruction() == 0 && inst->element_instruction()==0;

  std::string qualified_name = name;
  if (inst->ref_instruction())
  {
    qualified_name = cpp_type_of(inst);
  }

  if (top_level) {
    out_ << "const " << qualified_name << "::instruction_type*\n"
         << name << "::instruction()\n"
         << "{\n";
  }
  else
    add_to_instruction_list(name);

  prefixes_.push_back(name+"_element");


  if (inst->length_instruction()) {
    std::string context = gen_op_context(inst->length_instruction()->name(),
                                         inst->length_instruction()->op_context());

    // length
    out_ << "static uint32_field_instruction\n"
         << prefix_string() << name << "_length_instruction(\n"
         << "  0,"
         << "  " << get_operator_name(inst->length_instruction()) << ",\n"
         << "  " << get_presence(inst->length_instruction()) << ",\n"
         << "  " << inst->length_instruction()->id() << ", // id\n"
         << "  \""<< inst->length_instruction()->name() << "\", // name\n"
         << "  \""<< inst->length_instruction()->ns() << "\", // ns\n"
         << "  "<< context << ",  // opContext\n"
         << "  int_value_storage<uint32_t>(";
    if (!inst->length_instruction()->initial_value().is_empty())
      out_ << inst->length_instruction()->initial_value().get<uint32_t>() << "U";
    out_ <<  ")); // initial_value\n\n";
  }


  if (to_gen_subinstruction )
  {
    subinstructions_list_.resize(subinstructions_list_.size()+1);
    traverse(inst, "_element");
  }

  std::string lengthInstruction;

  if (inst->length_instruction()) {
    std::stringstream strm;
    strm << "&" << prefix_string()  << name <<  "_length_instruction";
    lengthInstruction = strm.str();
  }

  std::stringstream subinstruction_arg;
  if (to_gen_subinstruction )
  {
    output_subinstructions();

    subinstruction_arg << "  "<< prefix_string() << "subinstructions,\n"
                       << "  "<< inst->subinstructions_count() << ", // num_fields\n";
  }
  else {
    subinstruction_arg << "  0, // subinstructions\n"
                       << "  0, // subinstructions_count\n";
  }
  prefixes_.pop_back();

  if (top_level) {
    out_ << "const static " << name << "::instruction_type\n"
         << "  the_instruction(\n";
  }
  else {
    out_ << "const static " << cref_scope() << name << "_cref::instruction_type\n"
         << prefix_string() << name << "_instruction(\n";
  }

  out_ << "  "<<  index << ",\n"
       << "  " << get_presence(inst) << ",\n"
       << "  " << inst->id() << ", // id\n"
       << "  \"" << inst->name() << "\", // name\n"
       << "  \"" << inst->ns() << "\", // ns\n"
       << "  \"" << inst->dictionary() << "\", // dictionary\n"
       << subinstruction_arg.str()
       << "  "<< lengthInstruction << ", // length\n"
       << "  \"" << inst->typeref_name() << "\", // typeRef name \n"
       << "  \"" << inst->typeref_ns() << "\", // typeRef ns \n"
       << "  \"\", // cpp_ns\n";

  if (inst->element_instruction()) {
    out_ << "  " << cpp_type_of(inst->element_instruction()) << "::instruction(), // element_instruction\n";
  }
  else {
    out_ << "  0, // element_instruction\n";
  }

  if (inst->ref_instruction()) {
    out_ << "  " << cpp_type_of(inst->ref_instruction()) << "::instruction()); // ref_instruction\n\n";
  }
  else {
    out_ << "  0, // ref_instruction\n"
         << "  " << inst->tag() << "); //tag \n\n";
  }

  if (top_level) {
    out_ << "  return &the_instruction;\n"
         << "}\n\n";
  }
}

void cpp_gen::visit(const mfast::template_instruction* inst, void*)
{
  if (inst->subinstructions_count() == 0)
    return;

  std::string name( cpp_name( inst ) );

  prefixes_.push_back(name);

  subinstructions_list_.resize(subinstructions_list_.size()+1);

  out_ << "const " << name << "::instruction_type*\n"
       << name << "::instruction()\n"
       << "{\n";

  traverse(inst);

  output_subinstructions();
  prefixes_.pop_back();

  template_instructions_ << "  " << name << "::instruction(),\n";

  out_ << "  const static " << name << "::instruction_type the_instruction(\n"
       << "    " << inst->id() << ", // id\n"
       << "    \"" << inst->name() << "\", // name\n"
       << "    \""<< inst->ns() << "\", // ns\n"
       << "    \""<< inst->template_ns() << "\", // templateNs\n"
       << "    \""<< inst->dictionary() << "\", // dictionary\n"
       << "    "<< name << "__subinstructions,\n"
       << "    "<< inst->subinstructions_count() << ", // num_fields\n"
       << "    " << inst->has_reset_attribute() << ", // reset\n"
       << "  \"" << inst->typeref_name() << "\", // typeRef name \n"
       << "  \"" << inst->typeref_ns() << "\", // typeRef ns \n"
       << "  \"\", // cpp_ns\n"
       << "  " << inst->tag() << "); // tag \n\n"
       << "  return &the_instruction;\n"
       << "}\n\n";
}

void cpp_gen::visit(const mfast::templateref_instruction* inst, void*)
{
  std::size_t index = inst->field_index();

  out_ << "static const templateref_instruction\n"
       << prefix_string() << "templateref" << index << "_instruction(\n"
       << "  " << index << ", // index\n"
       << "  " << inst->tag() << "); // tag\n\n";

  std::stringstream tmp;
  tmp << "templateref" << index;
  add_to_instruction_list(tmp.str().c_str());
}

void cpp_gen::generate(mfast::dynamic_templates_description& desc)
{
  out_<< "#include \"" << filebase_ << ".h\"\n"
      << "\n"
      << "using namespace mfast;\n\n"
      << "namespace " << filebase_ << "\n{\n";

  this->traverse(desc);

  std::string instructions = template_instructions_.str();
  if (instructions.size())
    instructions.resize(instructions.size() - 2);

  out_ << "const template_instruction* "<< filebase_ << "_templates_instructions[] ={\n"
       << instructions
       << "};\n\n"
       << "mfast::templates_description* description()\n"
       << "{\n"
       << "  static mfast::templates_description desc(\n"
       << "    \"" << desc.ns()  << "\", // ns\n"
       << "    \"" << desc.template_ns()<< "\", // templateNs\n"
       << "    \"" << desc.dictionary() << "\", // dictionary\n"
       << "    " << filebase_ << "_templates_instructions);\n"
       << "  return &desc;\n"
       << "}\n\n"
       << "\n}\n";
}

void cpp_gen::visit(const mfast::enum_field_instruction* inst, void* top_level)
{
  std::string name( cpp_name( inst ) );
  std::size_t index = inst->field_index();
  std::string qualified_name = name;
  std::string instruction_variable_name;
  std::stringstream elements_variable_name;
  std::stringstream num_elements_name;
  std::stringstream instruction_type;
  std::string values_variable_name="0";

  if (inst->ref_instruction())
  {
    qualified_name = cpp_type_of(inst);
  }

  if (top_level) {
    out_ << "const " << qualified_name << "::instruction_type*\n"
         << name << "::instruction()\n"
         << "{\n";
    instruction_variable_name = "  the_instruction";
    instruction_type << qualified_name << "::instruction_type";
  }
  else {
    add_to_instruction_list(name);
    instruction_variable_name =  prefix_string() + name + "_instruction";
    instruction_type << cref_scope() << name << "_cref::instruction_type";
  }

  if (inst->ref_instruction()) {
    elements_variable_name << qualified_name << "::instruction()->elements()";
    num_elements_name << qualified_name << "::instruction()->num_elements()";
    values_variable_name = qualified_name + "::instruction()->element_values()";
    instruction_type.str(qualified_name + "::instruction_type");
  }
  else {
    elements_variable_name << "elements";
    num_elements_name << inst->num_elements();

    out_ << "static const char* elements[] = {\n";

    for (uint64_t i = 0; i < inst->num_elements(); ++i) {
      if (i != 0 ) {
        out_ << ",\n";
      }
      out_ << "  \"" << inst->elements()[i] << "\"";
    }
    out_ << "};\n";

    if (inst->element_values()) {
      out_ << "static const uint64_t values[] = {\n";

      for (uint64_t i = 0; i < inst->num_elements(); ++i) {
        if (i != 0 ) {
          out_ << ",\n";
        }
        out_ << "  " << inst->element_values()[i];
      }
      out_ << "};\n";
      values_variable_name = "values";
    }
  }

  std::string context = gen_op_context(inst->name(), inst->op_context());



  out_ << "const static " << instruction_type.str() << "\n"
       << instruction_variable_name << "(\n"
       << "  " << index << ",\n"
       << "  " << get_operator_name(inst) << ",\n"
       << "  " << get_presence(inst) << ",\n"
       << "  " << inst->id() << ", // id\n"
       << "  \""<< inst->name() << "\", // name\n"
       << "  \""<< inst->ns() << "\", // ns\n"
       << "  "<< context << ",  // opContext\n"
       << "  int_value_storage<uint64_t>(" << inst->initial_value().get<uint64_t>() <<  "), // initial_value\n"
       << "  " << elements_variable_name.str() << ", // element names\n"
       << "  " << values_variable_name << ", // element values\n"
       << "  " << num_elements_name.str() << ",// num elements\n"
       << "  0, // ref_instruction\n"
       << "  0, // cpp_ns\n"
       << "  " << inst->tag() << "); //tag \n\n";

  if (top_level) {
    out_ << "  return &the_instruction;\n"
         << "}\n\n";
  }
}
