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
#include "FastXML2Header.h"



class DependencyTracker
  : public FastXMLVisitor
{
  public:
    DependencyTracker(std::ostream& os, templates_registry_t& registry)
      : os_(os)
      , registry_(registry)
    {
    }

    virtual bool VisitTemplateRef(const XMLElement & element,
                                  const std::string& name_attr,
                                  std::size_t /* index */)
    {
      if (name_attr.size()) {
        std::string ns = get_optional_attr(element, "ns", current_context().ns_.c_str());
        templates_registry_t::iterator itr = registry_.find(ns + "||" + name_attr);
        if (itr != registry_.end()) {
          os_ << "#include \"" << itr->second << ".h\"\n";
        }
      }
      return true;
    }

  private:
    std::ostream& os_;
    templates_registry_t& registry_;
};

indent_t indent;


FastXML2Header::FastXML2Header(const char* filebase, templates_registry_t& registry)
  : FastCodeGenBase(filebase, ".h")
  , registry_(registry)
{
}

void FastXML2Header::restore_scope(const std::string& name_attr)
{
  // we need to remove the postfix "xxx_cref::" or "xxx_mref::"
  // from cref_scope_ and mref_scope_


  std::size_t substract_size =  name_attr.size() + sizeof("_cref::")-1;
  std::string str = cref_scope_.str();
  str.resize(str.size() - substract_size);
  cref_scope_.clear();
  cref_scope_.str(str);
}

/// Visit a document.
bool FastXML2Header::VisitEnter( const XMLDocument& doc)
{
  std::string filebase_upper = boost::to_upper_copy(filebase_);

  out_<< "#ifndef __" << filebase_upper << "_H__\n"
      << "#define __" << filebase_upper << "_H__\n"
      << "\n"
      << "#include <mfast.h>\n"
      << "#include <boost/array.hpp>\n";

  DependencyTracker tracker(out_, registry_);
  doc.Accept(&tracker);

  out_ << "namespace " << filebase_ << "\n{\n";
  return out_.good();
}

/// Visit a document.
bool FastXML2Header::VisitExit( const XMLDocument& /*doc*/ )
{
  std::string filebase_upper = boost::to_upper_copy(filebase_);

  out_<< "\n mfast::templates_description* description();\n\n"
      << "#include \"" << filebase_ << ".inl\"\n"
      << "}\n\n"
      << "#endif //__" << filebase_upper << "_H__\n";
  return out_.good();
}

bool FastXML2Header::VisitEnterTemplate (const XMLElement & /*element*/,
                                         const std::string& name_attr,
                                         std::size_t /* index */)
{
  header_cref_ << "\n"
               << indent << "class " << name_attr << "_cref\n"
               << indent << "  : public mfast::aggregate_cref\n"
               << indent << "{\n"
               << indent << "  public:\n"
               << indent << "    " << name_attr << "_cref(\n"
               << indent << "      const mfast::value_storage*           storage,\n"
               << indent << "      const mfast::group_field_instruction* instruction);\n\n"
               << indent << "    explicit " << name_attr << "_cref(const mfast::field_cref& other);\n\n";

  header_mref_ << "\n"
               << indent << "typedef mfast::make_aggregate_mref<" << name_attr << "_cref> " << name_attr << "_mref_base;\n"
               << indent << "class " << name_attr << "_mref\n"
               << indent << "  : public " << name_attr << "_mref_base\n"
               << indent << "{\n"
               << indent << "  public:\n"
               << indent << "    " << name_attr << "_mref(\n"
               << indent << "      mfast::allocator*                     alloc,\n"
               << indent << "      mfast::value_storage*                 storage,\n"
               << indent << "      const mfast::group_field_instruction* instruction);\n\n"
               << indent << "    explicit " << name_attr << "_mref(const mfast::field_mref_base& other);\n\n";

  cref_scope_ << name_attr << "_cref::";
  header_cref_.inc_indent(2);
  header_mref_.inc_indent(2);

  return true;
}

bool FastXML2Header::VisitExitTemplate (const XMLElement & element,
                                        const std::string& name_attr,
                                        std::size_t        numFields,
                                        std::size_t /* index */)
{
  header_cref_.dec_indent(2);
  header_mref_.dec_indent(2);

  header_cref_ << "\n"
               << indent << "};\n\n";
  header_mref_ << indent << "};\n\n";

  out_<< header_cref_.str() << header_mref_.str();

  header_cref_.clear();
  header_cref_.str("");
  header_mref_.clear();
  header_mref_.str("");

  out_<< "class " << name_attr << "\n"
      << "  : private boost::array<mfast::value_storage, " << numFields << ">\n"
      << "  , public mfast::message_type\n"
      << "{\n"
      << "  public:\n"
      << "    enum {\n"
      << "      the_id = " << get_optional_attr(element, "id", "0") << "\n"
      << "    };\n"
      << "    typedef mfast::template_instruction_ex<" << name_attr << "_cref> instruction_type;\n\n"
      << "    typedef mfast::make_message_cref<" << name_attr << "_cref, instruction_type> cref_type;\n"
      << "    typedef mfast::make_message_mref<" << name_attr << "_mref, instruction_type> mref_type;\n"
      << "    " <<   name_attr << "(\n"
      << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
      << "    " <<   name_attr << "(\n"
      << "      const " <<  name_attr << "_cref& other,\n"
      << "      mfast::allocator* alloc=mfast::malloc_allocator::instance());\n"
      << "    cref_type ref() const;\n"
      << "    cref_type cref() const;\n"
      << "    mref_type ref();\n"
      << "    mref_type mref();\n"
      << "    static const instruction_type* instruction();\n"
      << "  private:\n"
      << "    " << name_attr << "(const " << name_attr << "&);\n"
      << "    " << name_attr << "& operator = (const "  << name_attr << "&);\n"
      << "};\n\n";
  restore_scope(name_attr);

  // const XMLElement* typeRefElem = element.FirstChildElement("typeRef");
  // if (typeRefElem) {
  //   std::string typeRef_name = get_optional_attr(*typeRefElem, "name", "");
  //   if (typeRef_name.size()) {
  //     out_<< "typedef " << name_attr << " " << typeRef_name << ";\n"
  //         << "typedef " << name_attr << "_cref " << typeRef_name << "_cref;\n"
  //         << "typedef " << name_attr << "_mref " << typeRef_name << "_mref;\n";
  //   }
  // }

  return out_.good();
}

bool FastXML2Header::VisitEnterGroup (const XMLElement & element,
                                      const std::string& name_attr,
                                      std::size_t /* index */)
{
  if (only_child_templateRef(element) == 0)
  {
    std::string name = name_attr;

    header_cref_ << "\n"
                 << indent << "class " << name << "_cref\n"
                 << indent << "  : public mfast::group_cref\n"
                 << indent << "{\n"
                 << indent << "  public:\n"
                 << indent << "    typedef mfast::group_field_instruction instruction_type;\n"
                 << indent << "    typedef const instruction_type* instruction_cptr;\n"
                 << indent << "    " << name << "_cref(\n"
                 << indent << "      const mfast::value_storage*   storage,\n"
                 << indent << "      instruction_cptr              instruction);\n\n"
                 << indent << "    explicit " << name << "_cref(const mfast::field_cref& other);\n\n";


    header_mref_ << "\n"
                 << indent << "typedef mfast::make_group_mref<" << cref_scope_.str() << name << "_cref> " << name << "_mref_base;\n"
                 << indent << "class " << name << "_mref\n"
                 << indent << "  : public " << name << "_mref_base\n"
                 << indent << "{\n"
                 << indent << "  public:\n"
                 << indent << "    " << name_attr << "_mref(\n"
                 << indent << "      mfast::allocator*             alloc,\n"
                 << indent << "      mfast::value_storage*         storage,\n"
                 << indent << "      instruction_cptr              instruction\n\n"
                 << indent << "    explicit " << name_attr << "_mref(const mfast::field_mref_base& other);\n\n";

    cref_scope_ << name << "_cref::";
    header_cref_.inc_indent(2);
    header_mref_.inc_indent(2);
    return true;
  }
  return false;
}

bool FastXML2Header::VisitExitGroup (const XMLElement & element,
                                     const std::string& name_attr,
                                     std::size_t /* numFields */,
                                     std::size_t /* index */)
{
  const XMLElement* child = only_child_templateRef(element);
  if (child == 0) {
    header_cref_.dec_indent(2);
    header_mref_.dec_indent(2);

    header_cref_ << indent << "};\n\n";
    header_cref_ << indent << name_attr << "_cref get_" << name_attr << "() const;\n";

    header_mref_ << indent << "};\n\n";
    header_mref_ << indent << name_attr << "_mref set_" << name_attr << "() const;\n";

    restore_scope(name_attr);
  }
  else {
    const char* templateRef_name = child->Attribute("name", 0);
    std::string qulified_name = "mfast::group";
    if (templateRef_name) {
      std::string ns = get_optional_attr(*child, "ns", current_context().ns_.c_str());
      templates_registry_t::iterator itr = registry_.find(ns + "||" + templateRef_name);
      qulified_name = templateRef_name;
      if (itr != registry_.end()) {
        qulified_name = itr->second + "::" + templateRef_name;
      }
    }

    header_cref_ << indent << "typedef " << qulified_name << "_cref " << name_attr << "_cref;\n"
                 << indent << name_attr << "_cref get_" << name_attr << "() const;\n";
    header_mref_ << indent << "typedef " << qulified_name << "_mref " << name_attr << "_mref;\n"
                 << indent << name_attr << "_mref set_" << name_attr << "() const;\n";
  }
  return true;
}

bool FastXML2Header::VisitEnterSequence (const XMLElement & element,
                                         const std::string& name_attr,
                                         std::size_t /* index */)
{
  if (only_child(element) == 0)
  {
    std::string name = name_attr + "_element";
    header_cref_ << "\n"
                 << indent << "typedef mfast::sequence_element_cref " << name << "_cref_base;\n"
                 << indent << "class " << name << "_cref\n"
                 << indent << "  : public " << name << "_cref_base\n"
                 << indent << "{\n"
                 << indent << "  public:\n"
                 << indent << "    typedef mfast::sequence_instruction_ex<" << name << "_cref> instruction_type;\n"
                 << indent << "    typedef const instruction_type* instruction_cptr;\n"
                 << indent << "    " << name << "_cref(\n"
                 << indent << "      const mfast::value_storage* storage,\n"
                 << indent << "      instruction_cptr            instruction);\n\n";

    header_mref_ << "\n"
                 << indent << "typedef mfast::make_aggregate_mref<" << cref_scope_.str() << name << "_cref> " << name << "_mref_base;\n"
                 << indent << "class " << name << "_mref\n"
                 << indent << "  : public " << name << "_mref_base\n"
                 << indent << "{\n"
                 << indent << "  public:\n"
                 << indent << "    " << name << "_mref(\n"
                 << indent << "      mfast::allocator*     alloc,\n"
                 << indent << "      mfast::value_storage* storage,\n"
                 << indent << "      instruction_cptr      instruction);\n\n";


    cref_scope_ << name_attr << "_cref::";
    header_cref_.inc_indent(2);
    header_mref_.inc_indent(2);
    return true;
  }
  return false;
}

bool FastXML2Header::VisitExitSequence (const XMLElement & element,
                                        const std::string& name_attr,
                                        std::size_t /* numFields */,
                                        std::size_t /* index */)
{
  const XMLElement* child = only_child(element);
  if (child == 0) {
    header_cref_.dec_indent(2);
    header_mref_.dec_indent(2);

    header_cref_ << indent << "};\n\n"
                 << indent << "typedef mfast::make_sequence_cref<" << name_attr << "_element_cref> " << name_attr << "_cref;\n"
                 << indent << name_attr << "_cref get_" << name_attr << "() const;\n";

    header_mref_ << indent << "};\n\n"
                 << indent << "typedef mfast::make_sequence_mref<" << name_attr << "_element_mref> " << name_attr << "_mref;\n"
                 << indent << name_attr << "_mref set_" << name_attr << "() const;\n";

    restore_scope(name_attr);
  }
  else {

    if (strcmp(child->Name(), "templateRef") == 0) {
      const char* templateRef_name = child->Attribute("name", 0);
      if (templateRef_name) {
        std::string ns = get_optional_attr(*child, "ns", current_context().ns_.c_str());
        templates_registry_t::iterator itr = registry_.find(ns + "||" + templateRef_name);
        std::string qulified_name = templateRef_name;
        if (itr != registry_.end()) {
          qulified_name = itr->second + "::" + templateRef_name;
        }

        header_cref_ << indent << "typedef mfast::make_sequence_cref<" << qulified_name << "_cref> " << name_attr << "_cref;\n";
        header_mref_ << indent << "typedef mfast::make_sequence_mref<" << qulified_name << "_mref> " << name_attr << "_mref;\n";
      }
      else {
        header_cref_ << indent << "typedef mfast::sequence_cref " << name_attr << "_cref;\n";
        header_mref_ << indent << "typedef mfast::sequence_mref " << name_attr << "_mref;\n";
      }
    }
    else { // we have one element in the sequence
      const char* seq_element_type = child->Name();
      std::string cpp_type_name;
      
      if (strcmp(seq_element_type, "int32") == 0) {
        cpp_type_name = "int32";
      }
      else if (strcmp(seq_element_type, "uInt32") == 0) {
        cpp_type_name = "uint32";
      }
      else if (strcmp(seq_element_type, "int64") == 0) {
        cpp_type_name = "int64";
      }
      else if (strcmp(seq_element_type, "uInt64") == 0) {
        cpp_type_name = "uint64";
      }
      else if (strcmp(seq_element_type, "decimal") == 0) {
        cpp_type_name = "decimal";
      }
      else if (strcmp(seq_element_type, "string") == 0) {
        const char* charset = get_optional_attr(*element.FirstChildElement(), "charset", "ascii");
        cpp_type_name = charset;
        cpp_type_name += "_string";
      }
      else if (strcmp(seq_element_type, "byteVector") == 0) {
        cpp_type_name = "byte_vector";
      }
      
      header_cref_ << indent << "typedef mfast::make_sequence_cref<mfast::" << cpp_type_name << "_cref> " << name_attr << "_cref;\n";
      header_mref_ << indent << "typedef mfast::make_sequence_mref<mfast::" << cpp_type_name << "_mref> " << name_attr << "_mref;\n";
      
    }
  
    
    header_cref_ << indent << name_attr << "_cref get_" << name_attr << "() const;\n";
    header_mref_ << indent << name_attr << "_mref set_" << name_attr << "() const;\n";
  }
  return true;
}

bool FastXML2Header::VisitEnterSimpleValue (const XMLElement & element,
                                            const char*        cpp_type,
                                            const std::string& name_attr,
                                            std::size_t /* index */)
{
  header_cref_ << indent << "mfast::"<< cpp_type << "_cref get_" << name_attr << "() const;\n";
  if (!is_mandatory_constant(element)) {
    header_mref_ << indent << "mfast::"<< cpp_type << "_mref set_" << name_attr << "() const;\n";
  }
  return true;
}

bool FastXML2Header::VisitString (const XMLElement & element, const std::string& name_attr, std::size_t index)
{
  const char* charset = get_optional_attr(element, "charset", "ascii");
  std::string cpp_type = charset;
  cpp_type += "_string";
  VisitEnterSimpleValue(element, cpp_type.c_str(), name_attr, index);
  return true;
}

bool FastXML2Header::VisitInteger (const XMLElement & element,
                                   int                bits,
                                   const std::string& name_attr,
                                   std::size_t        index)
{
  char buf[10];
  TIXML_SNPRINTF(buf, 10, "uint%d",bits);
  const char* cpp_type = (element.Name()[0] == 'u') ? buf : buf+1;
  VisitEnterSimpleValue(element,
                        cpp_type,
                        name_attr,
                        index);
  return true;
}

bool FastXML2Header::VisitDecimal (const XMLElement & element, const std::string& name_attr, std::size_t index)
{
  VisitEnterSimpleValue(element, "decimal", name_attr, index);
  return true;
}

bool FastXML2Header::VisitByteVector (const XMLElement & element, const std::string& name_attr, std::size_t index)
{
  VisitEnterSimpleValue(element, "byte_vector", name_attr, index);
  return true;
}

bool FastXML2Header::VisitTemplateRef(const XMLElement & element, const std::string& name_attr, std::size_t index)
{
  if (name_attr.size()) {
    std::string ns = get_optional_attr(element, "ns", current_context().ns_.c_str());
    templates_registry_t::iterator itr = registry_.find(ns + "||" + name_attr);
    std::string qulified_name = name_attr;
    if (itr != registry_.end()) {
      qulified_name = itr->second + "::" + name_attr;
    }
    header_cref_ << indent << qulified_name << "_cref get_" << name_attr << "() const;\n";
    header_mref_ << indent << qulified_name << "_mref set_" << name_attr << "() const;\n";
  }
  else {
    header_cref_ << indent << "mfast::nested_message_cref get_nested_message" << index << "() const;\n";
    header_mref_ << indent << "mfast::nested_message_mref set_nested_message" << index << "() const;\n";
  }
  return true;
}

bool FastXML2Header::VisitEnterDefine(const XMLElement &, const std::string& name_attr)
{
  defined_name_ = name_attr;
  return true;
}

bool FastXML2Header::VisitExitDefine(const XMLElement &, const std::string& )
{
  defined_name_.clear();
  return true;
}
