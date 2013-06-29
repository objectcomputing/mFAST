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
#include "FastXML2Source.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
namespace {


// TODO: check mandatory field with default operator must have initial value

struct cstr_compare
{
  bool operator()(const char* lhs, const char* rhs) const
  {
    return strcmp(lhs, rhs) < 0;
  }

};

struct map_value_type
{
  const char* first;
  const char* second;
};


struct is_space {
  bool operator() (char c) const
  {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      return true;
    }
    return false;
  }

};

struct decimal_value {
  int64_t mantissa;
  int32_t exponent;
};

inline std::ostream&
operator << (std::ostream & out, const decimal_value& in)
{
  out << in.mantissa << "LL, " << in.exponent;
  return out;
}

std::istream &
operator>>(std::istream &source, decimal_value& result)
{
  std::string decimal_string;
  source >> decimal_string;

  std::size_t float_pos = decimal_string.find_first_of('.');
  std::size_t nonzero_pos = decimal_string.find_last_not_of(".0");

  if (float_pos == std::string::npos || nonzero_pos < float_pos) {
    // this is an integer
    if (float_pos != std::string::npos) {
      decimal_string = decimal_string.substr(0, float_pos);
      nonzero_pos = decimal_string.find_last_not_of('0');
    }
    if (nonzero_pos == std::string::npos)
      nonzero_pos = 0;
    result.exponent = decimal_string.size()-1 - nonzero_pos;
    result.mantissa = boost::lexical_cast<int64_t>(decimal_string.substr(0, nonzero_pos+1));
  }
  else {
    decimal_string = decimal_string.substr(0, nonzero_pos+1);
    result.exponent = float_pos - decimal_string.size() +1;
    result.mantissa = boost::lexical_cast<int64_t>(decimal_string.erase(float_pos, 1));
  }
  return source;
}

class XMLFormatError
  : public std::exception
{
  static char buffer_[256];

  public:
    XMLFormatError(const char* format, ...)
    {
      va_list ap;
      va_start(ap, format);
      vsnprintf(buffer_, 256, format, ap);
      va_end(ap);
      buffer_[255] = 0;
    }

    const char* what() const throw ()
    {
      return buffer_;
    }

};

char XMLFormatError::buffer_[256];
}


void FastXML2Source::restore_scope(const std::string& name_attr)
{
  // we need to remove the postfix "xxx_cref::" or "xxx_mref::"
  // from cref_scope_ and mref_scope_
  std::size_t substract_size =  name_attr.size() + sizeof("_cref::")-1;
  std::string str = cref_scope_.str();
  str.resize(str.size() - substract_size);
  cref_scope_.clear();
  cref_scope_.str(str);
}

FastXML2Source::FastXML2Source(const char* filebase, templates_registry_t& registry)
  : FastCodeGenBase(filebase, ".cpp")
  , registry_(registry)
{
}

/// Visit a document.
bool FastXML2Source::VisitEnter( const XMLDocument& /*doc*/ )
{
  out_<< "#include \"" << filebase_ << ".h\"\n"
      << "\n"
      << "using namespace mfast;\n\n"
      << "namespace " << filebase_ << "\n{\n";
  return out_.good();
}

/// Visit a document.
bool FastXML2Source::VisitExit( const XMLDocument& /*doc*/ )
{
  out_ << "\n}\n";
  return out_.good();
}

bool FastXML2Source::VisitExitTemplates (const XMLElement & element,
                                         std::size_t        numFields)
{
  std::string instructions = template_instructions_.str();
  if (instructions.size())
    instructions.resize(instructions.size() - 2);

  out_ << "const template_instruction* "<< filebase_ << "_templates_instructions[] ={\n"
       << instructions
       << "};\n\n";

  out_ << "mfast::templates_description the_description(\n"
       << "  \"" << get_optional_attr(element, "ns", "")  << "\", // ns\n"
       << "  \"" << get_optional_attr(element, "templateNs", "")<< "\", // templateNs\n"
       << "  \"" << get_optional_attr(element, "dictionary", "") << "\", // dictionary\n"
       << "  " << filebase_ << "_templates_instructions);\n";


  return out_.good();
}

const XMLElement* FastXML2Source::fieldOpElement(const XMLElement & element)
{
  static const char* field_op_names[] = {
    "constant","default","copy","increment","delta","tail"
  };

  static std::set<const char*, cstr_compare> field_op_set (field_op_names, field_op_names + 6);

  for (const XMLElement* child = element.FirstChildElement(); child != 0; child = child->NextSiblingElement())
  {
    if (field_op_set.count(child->Name())) {
      return child;
    }
  }
  return 0;
}

void FastXML2Source::add_to_instruction_list(const std::string & name_attr)
{
  std::stringstream strm;
  strm << "  &" << name_attr << "_instruction,\n";
  subinstructions_list_.back() += strm.str();
}

bool FastXML2Source::VisitEnterTemplate (const XMLElement & element,
                                         const std::string& name_attr,
                                         std::              size_t /* index */)
{
  std::string qulified_name = current_context().ns_ + "||" + name_attr;
  registry_[qulified_name] = filebase_;

  out_ << "namespace " << name_attr << "_def\n"
       << "{\n";

  subinstructions_list_.resize(subinstructions_list_.size()+1);
  cref_scope_ << name_attr << "_cref::";

  return out_.good();
}

bool FastXML2Source::output_typeref(const XMLElement & element)
{
  std::string typeRef_name;
  std::string typeRef_ns;

  const XMLElement* typeRefElem = element.FirstChildElement("typeRef");
  if (typeRefElem) {
    typeRef_name = get_optional_attr(*typeRefElem, "name", "");
    typeRef_ns = get_optional_attr(*typeRefElem, "ns", "");
  }

  out_ << "  \"" << typeRef_name << "\", // typeRef name \n"
       << "  \"" << typeRef_ns << "\"); // typeRef ns \n\n";

  return out_.good();
}

bool FastXML2Source::VisitExitTemplate (const XMLElement & element,
                                        const std::string& name_attr,
                                        std::size_t        numFields,
                                        std::              size_t /* index */)
{
  restore_scope(name_attr);

  output_subinstructions(name_attr);

  bool reset = false;
  const XMLAttribute* reset_attr = element.FindAttribute("scp:reset");
  if (reset_attr == 0)
    reset_attr = element.FindAttribute("reset");

  if (reset_attr) {
    if (strcmp(reset_attr->Value(), "true") == 0 || strcmp(reset_attr->Value(), "yes") == 0)
      reset = true;
  }

  out_ << "const " << name_attr << "_cref::instruction_type\n"
       << name_attr << "_cref::the_instruction(\n"
       << "  " << get_optional_attr(element, "id", "0") << ", // id\n"
       << "  \"" << name_attr << "\", // name\n"
       << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
       << "  \""<< get_optional_attr(element, "templateNs", "") << "\", // templateNs\n"
       << "  \""<< get_optional_attr(element, "dictionary", "") << "\", // dictionary\n"
       << "  "<< name_attr << "_def::subinstructions,\n"
       << "  "<< numFields << ", // fields_count\n"
       << "  " << reset << ", // reset\n";


  template_instructions_ << "  &" << name_attr << "_cref::the_instruction,\n";
  return output_typeref(element);
}

bool FastXML2Source::VisitEnterGroup (const XMLElement & element,
                                      const std::string& name_attr,
                                      std::size_t        index)
{
  cref_scope_ << name_attr << "_cref::";
  out_ << "namespace " << name_attr << "_def\n"
       << "{\n";

  add_to_instruction_list(name_attr);
  subinstructions_list_.resize(subinstructions_list_.size()+1);

  return out_.good();
}

bool FastXML2Source::VisitExitGroup (const XMLElement & element,
                                     const std::string& name_attr,
                                     std::size_t        numFields,
                                     std::size_t        index)
{
  output_subinstructions(name_attr);

  out_ << cref_scope_.str() << "instruction_type\n"
       << name_attr << "_instruction(\n"
       << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
       << "  "<< get_optional_attr(element, "id", "0") << ", // id\n"
       << "  \""<< name_attr << "\", // name\n"
       << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
       << "  \""<< get_optional_attr(element, "dictionary", "") << "\", // dictionary\n"
       << "  "<< name_attr << "_def::subinstructions,\n"
       << "  "<< numFields << ", // fields_count\n";

  restore_scope(name_attr);
  return output_typeref(element);
}

bool FastXML2Source::VisitEnterSequence (const XMLElement & element,
                                         const std::string& name_attr,
                                         std::size_t        index)
{
  cref_scope_ << name_attr << "_element_cref::";
  out_ << "namespace " << name_attr << "_def\n"
       << "{\n";

  add_to_instruction_list(name_attr);
  subinstructions_list_.resize(subinstructions_list_.size()+1);

  const XMLElement* length_element = element.FirstChildElement("length");

  std::string fieldOpName;
  std::string opContext;
  std::string initialValue;
  std::string id;
  std::string length_name;
  std::string ns;

  if (length_element) {
    get_field_attributes(*length_element, name_attr, fieldOpName, opContext, initialValue);

    length_name = get_optional_attr(*length_element, "name", "");
    id = get_optional_attr(*length_element, "id", "0");
    ns = get_optional_attr(*length_element, "ns", "");

    if (initialValue.size())
      initialValue += "U"; // add unsigned integer suffix
  }

  // length
  out_ << "uint32_field_instruction\n"
       << name_attr << "_length_instruction(\n"
       << "  "<< "operator_" << fieldOpName << ",\n"
       << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
       << "  "<< id << ", // id\n"
       << "  \""<< length_name << "\", // name\n"
       << "  \""<< ns << "\", // ns\n"
       << "  "<< opContext << ",  // opContext\n"
       << "  nullable<uint32_t>(" << initialValue << ")); // initial_value\n\n";
  return out_.good();
}

bool FastXML2Source::VisitExitSequence (const XMLElement & element,
                                        const std::string& name_attr,
                                        std::size_t        numFields,
                                        std::size_t        index)
{

  output_subinstructions(name_attr);

  std::stringstream strm;
  strm << "&" << name_attr << "_def::"  << name_attr <<  "_length_instruction";
  std::string lengthInstruction = strm.str();


  out_ << cref_scope_.str() << "instruction_type\n"
       << name_attr << "_instruction(\n"
       << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
       << "  "<< get_optional_attr(element, "id", "0") << ", // id\n"
       << "  \""<< name_attr << "\", // name\n"
       << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
       << "  \""<< get_optional_attr(element, "dictionary", "") << "\", // dictionary\n"
       << "  "<< name_attr << "_def::subinstructions,\n"
       << "  "<< numFields << ", // fields_count\n"
       << "  "<< lengthInstruction << ", // length\n";
  restore_scope(name_attr);
  return output_typeref(element);
}

bool FastXML2Source::get_field_attributes(const XMLElement & element,
                                          const std::string& name_attr,
                                          std::string&       fieldOpName,
                                          std::string&       opContext,
                                          std::string&       initialValue)
{
  fieldOpName = "none";
  opContext = "0";
  initialValue = "";

  const XMLElement* fieldOpElem = fieldOpElement(element);

  if (fieldOpElem) {
    fieldOpName = fieldOpElem->Name();

    std::string opContext_key = get_optional_attr(*fieldOpElem, "key", "");
    if (!opContext_key.empty()) {

      out_ << "op_context_t " << name_attr << "_opContext ={\n"
           << "  \"" << opContext_key << "\", \n"
           << "  \""<< get_optional_attr(*fieldOpElem, "ns", "") << "\", \n"
           << "  \""<< get_optional_attr(*fieldOpElem, "dictionary", "") << "\"};";

      opContext = "&";
      opContext += name_attr;
      opContext += "_opContext";
    }
    initialValue = get_optional_attr(*fieldOpElem, "value", "");
    return true;
  }
  return false;
}

bool FastXML2Source::VisitString (const XMLElement & element, const std::string& name_attr, std::size_t index)
{
  std::string fieldOpName;
  std::string opContext;
  std::string initialValue;

  get_field_attributes(element, name_attr, fieldOpName, opContext, initialValue);

  std::string charset =  get_optional_attr(element, "charset", "ascii");
  out_ << charset << "_field_instruction\n"
       << name_attr << "_instruction(\n"
       << "  "<< "operator_" << fieldOpName << ",\n"
       << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
       << "  "<< get_optional_attr(element, "id", "0") << ", // id\n"
       << "  \""<< name_attr << "\", // name\n"
       << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
       << "  "<< opContext;


  if (initialValue.size()) {
    out_ << ",  // opContext\n"
         << "  "<< "\""<< initialValue << "\", // initial_value\n"
         << "  "<<  initialValue.size() << "); // intitial_value length\n\n";
  }
  else {
    out_ << ");  // opContext\n\n";
  }

  add_to_instruction_list(name_attr);
  return out_.good();
}

bool FastXML2Source::VisitInteger (const XMLElement & element,
                                   int                bits,
                                   const std::string& name_attr,
                                   std::size_t        index)
{
  std::string fieldOpName;
  std::string opContext;
  std::string initialValue;

  get_field_attributes(element, name_attr, fieldOpName, opContext, initialValue);

  bool is_unsigned = element.Name()[0] == 'u';

  char buf[10];
  TIXML_SNPRINTF(buf, 10, "uint%d",bits);
  const char* cpp_type = is_unsigned ? buf : buf+1;

  if (initialValue.size()) {
    const char* unsigned_suffix = is_unsigned ? "U" : "";
    const char* longlong_suffix = (bits == 64) ? "LL" : "";
    initialValue += unsigned_suffix;
    initialValue += longlong_suffix;
  }

  out_ << cpp_type << "_field_instruction\n"
       << name_attr << "_instruction(\n"
       << "  "<< "operator_" << fieldOpName << ",\n"
       << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
       << "  "<< get_optional_attr(element, "id", "0") << ", // id\n"
       << "  \""<< name_attr << "\", // name\n"
       << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
       << "  "<< opContext << ",  // opContext\n"
       << "  nullable<"<< cpp_type << "_t>(" << initialValue  <<  ")); // initial_value\n\n";

  add_to_instruction_list(name_attr);
  return out_.good();
}

bool FastXML2Source::VisitDecimal (const XMLElement & element,
                                   const std::string& name_attr,
                                   std::size_t        index)
{
  const XMLElement* mantissa_element = element.FirstChildElement("mantissa");
  const XMLElement* exponent_element = element.FirstChildElement("exponent");

  if (mantissa_element || exponent_element) {

    std::string mantissa_fieldOpName;
    std::string mantissa_opContext;
    std::string mantissa_initialValue;

    get_field_attributes(*mantissa_element, name_attr + "_mantissa",
                         mantissa_fieldOpName,
                         mantissa_opContext,
                         mantissa_initialValue);

    std::string exponent_fieldOpName;
    std::string exponent_opContext;
    std::string exponent_initialValue;

    if (mantissa_initialValue.size())
      mantissa_initialValue += "LL";

    out_ << "mantissa_field_instruction\n"
         << name_attr << "_mantissa_instruction(\n"
         << "  "<< "operator_" << mantissa_fieldOpName << ",\n"
         << "  "<< mantissa_opContext << ",  // mantissa opContext\n"
         << "  nullable<int64_t>("<< mantissa_initialValue << "));// mantissa inital value\n\n";

    get_field_attributes(*exponent_element, name_attr + "_exponent",
                         exponent_fieldOpName,
                         exponent_opContext,
                         exponent_initialValue);

    out_ << "decimal_field_instruction\n"
         << name_attr << "_instruction(\n"
         << "  "<< "operator_" << exponent_fieldOpName << ", // exponent\n"
         << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
         << "  "<< get_optional_attr(element, "id", "0") << ", // id\n"
         << "  \""<< name_attr << "\", // name\n"
         << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
         << "  "<< exponent_opContext << ",  // exponent opContext\n"
         << "  &" << name_attr << "_mantissa_instruction,\n"
         << "  nullable<int8_t>("<< exponent_initialValue << ")); // exponent intitial_value\n";

  }
  else {

    std::string fieldOpName;
    std::string opContext;
    std::string initialValue;

    get_field_attributes(element, name_attr, fieldOpName, opContext, initialValue);

    if (initialValue.size()) {
      std::stringstream strm;
      strm << boost::lexical_cast<decimal_value>(initialValue);
      initialValue = strm.str();
    }

    try {

      out_ << "decimal_field_instruction\n"
           << name_attr << "_instruction(\n"
           << "  "<< "operator_" << fieldOpName << ",\n"
           << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
           << "  "<< get_optional_attr(element, "id", "0") << ", // id\n"
           << "  \""<< name_attr << "\", // name\n"
           << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
           << "  "<< opContext << ",  // opContext\n"
           << "  nullable_decimal("<< initialValue  << ")); // intitial_value\n\n";

    }
    catch (boost::bad_lexical_cast&) {
      throw XMLFormatError("%s is not a valid decimal number", initialValue.c_str());
    }
  }

  add_to_instruction_list(name_attr);
  return out_.good();
}

bool FastXML2Source::VisitByteVector (const XMLElement & element,
                                      const std::string& name_attr,
                                      std::size_t        index)
{
  std::string fieldOpName;
  std::string opContext;
  std::string initialValue;

  get_field_attributes(element, name_attr, fieldOpName, opContext, initialValue);

  if (initialValue.size()) {
    // remove all spaces
    std::string old = initialValue;
    initialValue.erase( std::remove_if(initialValue.begin(), initialValue.end(), is_space()), initialValue.end());
    initialValue.find_first_not_of("0123456789ABCDEFabcdef");
    std::size_t invalid_pos = initialValue.find_first_not_of("0123456789ABCDEFabcdef");

    if (invalid_pos != std::string::npos) {
      throw XMLFormatError("\"%s\" is not a valid byte vector, invalid character %c\n", old.c_str(), initialValue[invalid_pos] );
    }
    else if (initialValue.size() % 2) {
      throw XMLFormatError("\"%s\" is not a valid byte vector, it must have even digits\n", old.c_str());
    }

    std::string hex_string;
    hex_string.reserve(2* initialValue.size());

    std::string::const_iterator itr = initialValue.begin();

    while (itr != initialValue.end()) {
      const char* prefix = "\\x";
      hex_string.insert(hex_string.end(), prefix, prefix+2);
      hex_string.insert(hex_string.end(), itr, itr+2);
      itr += 2;
    }
    initialValue = hex_string;
  }

  out_ << "byte_vector_field_instruction\n"
       << name_attr << "_instruction(\n"
       << "operator_" << fieldOpName << ",\n"
       << "  "<< "presence_" << get_optional_attr(element, "presence", "mandatory") << ",\n"
       << "  "<< get_optional_attr(element, "id", "0") << ", // id\n"
       << "  \""<< name_attr << "\", // name\n"
       << "  \""<< get_optional_attr(element, "ns", "") << "\", // ns\n"
       << "  "<< opContext << ",  // opContext\n";

  if (initialValue.size()) {
    out_ << "  \""<< initialValue << "\", // initial_value\n"
         << "  " <<  initialValue.size()/4 << ", // intitial_value length\n";
  }

  const XMLElement* length_element = element.FirstChildElement("length");
  if (length_element) {
    out_ << get_optional_attr(*length_element, "id", "0") << ", // length id\n"
         << get_optional_attr(*length_element, "name", "0") << ", // length name\n"
         << get_optional_attr(*length_element, "ns", "0") << "); // length ns\n\n";
  }
  else {
    out_ << "0,0,0}; // no length element\n\n";
  }

    add_to_instruction_list(name_attr);
  return out_.good();
}

void FastXML2Source::output_subinstructions(const std::string name_attr)
{
  std::string content = subinstructions_list_.back();
  content.resize(content.size()-1);

  out_ << "const field_instruction* subinstructions[] = {\n"
       << content << "\n"
       << "};\n\n"
       << "} // namespace " << name_attr << "\n\n";
  subinstructions_list_.pop_back();
}

bool FastXML2Source::VisitTemplateRef(const XMLElement & element, const std::string& name_attr, std::size_t index)
{
  if (name_attr.size()) {
    std::string ns = get_optional_attr(element, "ns", current_context().ns_.c_str());
    std::string cpp_namespace;

    templates_registry_t::const_iterator itr = registry_.find(ns+"||"+name_attr);
    if (itr != registry_.end()) {
      if (itr->second != filebase_) {
        cpp_namespace = itr->second + "::";
      }
    }
    else {
      std::stringstream err;
      err << "Error: Cannot find the definition for static templateRef name=\""
                << name_attr << "\", ns=\"" << ns << "\"\n";
      throw std::runtime_error(err.str());
    }
    std::string tmp = cpp_namespace + name_attr;
    tmp += "_cref::the";
    add_to_instruction_list(tmp);
  }
  else {
    subinstructions_list_.back() += "templateref_instruction::instance()";
  }
  return true;
}
