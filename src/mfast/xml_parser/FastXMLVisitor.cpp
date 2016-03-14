// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "FastXMLVisitor.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
const char *FastXMLVisitor::get_optional_attr(const XMLElement &element,
                                              const char *attr_name,
                                              const char *default_value) const {
  const XMLAttribute *attr = element.FindAttribute(attr_name);
  if (attr == nullptr) {
    return default_value;
  }
  return attr->Value();
}

bool FastXMLVisitor::is_mandatory_constant(const XMLElement &element) {
  if (strcmp("mandatory",
             get_optional_attr(element, "presence", "mandatory")) == 0) {
    if (element.FirstChildElement("constant")) {
      return true;
    }
  }
  return false;
}

FastXMLVisitor::FastXMLVisitor() { num_fields_.push_back(0); }

bool FastXMLVisitor::VisitEnterTemplates(const XMLElement & /* element */) {
  return true;
}

bool FastXMLVisitor::VisitExitTemplates(const XMLElement & /* element */,
                                        std::size_t /* numFields */) {
  return true;
}

bool FastXMLVisitor::VisitEnterTemplate(const XMLElement & /* element */,
                                        const std::string & /* name_attr */,
                                        std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitExitTemplate(const XMLElement & /* element */,
                                       const std::string & /* name_attr */,
                                       std::size_t /* numFields */,
                                       std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitTemplateRef(const XMLElement & /* element */,
                                      const std::string & /* name_attr */,
                                      std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitEnterGroup(const XMLElement & /* element */,
                                     const std::string & /* name_attr */,
                                     std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitExitGroup(const XMLElement & /* element */,
                                    const std::string & /* name_attr */,
                                    std::size_t /* numFields */,
                                    std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitEnterSequence(const XMLElement & /* element */,
                                        const std::string & /* name_attr */,
                                        std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitExitSequence(const XMLElement & /* element */,
                                       const std::string & /* name_attr */,
                                       std::size_t /* numFields */,
                                       std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitInteger(const XMLElement & /* element */,
                                  int /* bits */,
                                  const std::string & /* name_attr */,
                                  std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitDecimal(const XMLElement & /* element */,
                                  const std::string & /* name_attr */,
                                  std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitString(const XMLElement & /* element */,
                                 const std::string & /* name_attr */,
                                 std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitByteVector(const XMLElement & /* element */,
                                     const std::string & /* name_attr */,
                                     std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitIntVector(const XMLElement & /* element */,
                                    int /* bits */,
                                    const std::string & /* name_attr */,
                                    std::size_t /* index */) {
  return true;
}

bool FastXMLVisitor::VisitEnterDefine(const XMLElement & /* element */,
                                      const std::string & /* name_attr */) {
  return true;
}

bool FastXMLVisitor::VisitExitDefine(const XMLElement & /* element */,
                                     const std::string & /* name_attr */) {
  return true;
}

void FastXMLVisitor::save_context(const XMLElement &element) {
  context_stack_.push_back(context_stack_.back());
  instruction_context &context = context_stack_.back();
  context.ns_ = get_optional_attr(element, "ns", context.ns_.c_str());
  context.dictionary_ =
      get_optional_attr(element, "dictionary", context.dictionary_.c_str());
}

bool parse_bits(const char *str, int &bits, bool &is_vector) {
  bits = 0;
  if (str[0] == '6' && str[1] == '4') {
    bits = 64;
  } else if (str[0] == '3' && str[1] == '2') {
    bits = 32;
  }

  if (bits != 0) {
    is_vector = std::strcmp(str + 2, "Vector") == 0;
  }

  return bits != 0;
}

bool FastXMLVisitor::VisitEnter(const XMLElement &element,
                                const XMLAttribute * /* attr */) {
  const char *element_name = element.Name();

  if (strcmp(element_name, "templates") == 0) {
    instruction_context context;
    context.ns_ = get_optional_attr(element, "templateNs", "");
    context.dictionary_ = get_optional_attr(element, "dictionary", "");

    context_stack_.push_back(context);
    return VisitEnterTemplates(element);
  }

  std::string name_attr = get_optional_attr(element, "name", "");

  if (strcmp(element_name, "templateRef") == 0) {
    return VisitTemplateRef(element, name_attr, num_fields_.back());
  }

  bool result = true;

  if (strcmp(element_name, "template") == 0) {
    save_context(element);
    result = VisitEnterTemplate(element, name_attr, num_fields_.back());
  } else if (strcmp(element_name, "group") == 0) {
    save_context(element);
    result = VisitEnterGroup(element, name_attr, num_fields_.back());
  } else if (strcmp(element_name, "sequence") == 0) {
    save_context(element);
    result = VisitEnterSequence(element, name_attr, num_fields_.back());
  } else if (strcmp(element_name, "define") == 0) {
    return VisitEnterDefine(element, name_attr);
  } else {
    int bits;
    bool is_vector;
    if ((strncmp(element_name, "int", 3) == 0 &&
         parse_bits(element_name + 3, bits, is_vector)) ||
        (strncmp(element_name, "uInt", 4) == 0 &&
         parse_bits(element_name + 4, bits, is_vector))) {
      // int8, int16 and uint8, uint16 are not standards, convert them to int32
      // and uint32 respectively
      if (is_vector)
        return VisitIntVector(element, bits, name_attr, num_fields_.back());
      return VisitInteger(element, bits, name_attr, num_fields_.back());
    } else if (strcmp(element_name, "decimal") == 0) {
      return VisitDecimal(element, name_attr, num_fields_.back());
    } else if (strcmp(element_name, "string") == 0) {
      return VisitString(element, name_attr, num_fields_.back());
    } else if (strcmp(element_name, "byteVector") == 0) {
      return VisitByteVector(element, name_attr, num_fields_.back());
    }
    return true;
  }
  num_fields_.push_back(0);
  return result;
}

bool FastXMLVisitor::VisitExit(const XMLElement &element) {

  const char *element_name = element.Name();

  bool result = true;

  if (strcmp(element_name, "templates") == 0) {
    return VisitExitTemplates(element, num_fields_.back());
  }

  std::string name_attr = get_optional_attr(element, "name", "");

  if (strcmp(element_name, "templateRef") == 0) {
    num_fields_.back() += 1;
  }

  if (name_attr.empty())
    return true;

  typedef bool (FastXMLVisitor::*VisitExitPtr)(
      const XMLElement &, const std::string &, std::size_t, std::size_t index);
  VisitExitPtr member_ptr;

  if (strcmp(element_name, "template") == 0) {
    member_ptr = &FastXMLVisitor::VisitExitTemplate;
  } else if (strcmp(element_name, "group") == 0) {
    member_ptr = &FastXMLVisitor::VisitExitGroup;
  } else if (strcmp(element_name, "sequence") == 0) {
    member_ptr = &FastXMLVisitor::VisitExitSequence;
  } else if (strcmp(element_name, "define") == 0) {
    return VisitExitDefine(element, name_attr);
  } else if (strncmp(element_name, "int", 3) == 0 ||
             strncmp(element_name, "uInt", 4) == 0 ||
             strcmp(element_name, "decimal") == 0 ||
             strcmp(element_name, "string") == 0 ||
             strcmp(element_name, "byteVector") == 0) {
    num_fields_.back() += 1;
    return true;
  } else {
    return true;
  }

  std::size_t numFields = num_fields_.back();
  num_fields_.pop_back();
  result =
      (this->*member_ptr)(element, name_attr, numFields, num_fields_.back());
  num_fields_.back() += 1;
  context_stack_.pop_back();

  return result;
}

FastXMLVisitor::instruction_context &FastXMLVisitor::current_context() {
  return context_stack_.back();
}

const XMLElement *FastXMLVisitor::only_child(const XMLElement &element) {
  const XMLElement *first_elem = element.FirstChildElement();
  if (strcmp(first_elem->Name(), "length") == 0) {
    first_elem = first_elem->NextSiblingElement();
  }
  if (first_elem->NextSibling() == nullptr)
    return first_elem;
  return nullptr;
}

// if element has only child and the child element is templateRef, it returns
// the address of the child element; otherwise it returns 0.
const XMLElement *
FastXMLVisitor::only_child_templateRef(const XMLElement &element) {
  const XMLElement *first_elem = only_child(element);
  if (first_elem && strcmp(first_elem->Name(), "templateRef") == 0)
    return first_elem;
  return nullptr;
}
