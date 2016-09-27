// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "tinyxml2.h"
#include "../instructions/field_instruction.h"
#include <cstring>
#include <boost/lexical_cast.hpp>

namespace mfast {
namespace xml_parser {
using namespace tinyxml2;
struct fast_xml_attributes {
  const char *name_;
  const char *id_;
  const char *ns_;
  const char *templateNs_;
  const char *dictionary_;
  const char *presence_;
  const char *charset_;
  const char *tag_;

  fast_xml_attributes()
      : name_(nullptr), id_(nullptr), ns_(nullptr), templateNs_(nullptr),
        dictionary_(nullptr), presence_(nullptr), charset_(nullptr),
        tag_(nullptr) {}

  fast_xml_attributes(const char *name)
      : name_(name), id_(nullptr), ns_(nullptr), templateNs_(nullptr),
        dictionary_(nullptr), presence_(nullptr), charset_(nullptr),
        tag_(nullptr) {}

  fast_xml_attributes(const XMLAttribute *attr) {
    name_ = nullptr;
    id_ = nullptr;
    ns_ = nullptr;
    templateNs_ = nullptr;
    dictionary_ = nullptr;
    presence_ = nullptr;
    charset_ = nullptr;
    tag_ = nullptr;

    set(attr);
  }

  void set(const XMLAttribute *attr) {
    while (attr != nullptr) {
      const char *name = attr->Name();

      if (std::strcmp(name, "name") == 0)
        name_ = attr->Value();
      else if (std::strcmp(name, "ns") == 0)
        ns_ = attr->Value();
      else if (std::strcmp(name, "templateNs") == 0)
        templateNs_ = attr->Value();
      else if (std::strcmp(name, "dictionary") == 0)
        dictionary_ = attr->Value();
      else if (std::strcmp(name, "id") == 0)
        id_ = attr->Value();
      else if (std::strcmp(name, "presence") == 0)
        presence_ = attr->Value();
      else if (std::strcmp(name, "charset") == 0)
        charset_ = attr->Value();
      else if (std::strcmp(name, "mfast:tag") == 0)
        tag_ = attr->Value();
      attr = attr->Next();
    }
  }

  presence_enum_t get_presence(const field_instruction *inst) const {
    bool is_optional = (presence_ == nullptr)
                           ? inst->optional()
                           : (std::strcmp(presence_, "optional") == 0);
    return is_optional ? presence_optional : presence_mandatory;
  }

  uint32_t get_id(const field_instruction *inst) const {
    return id_ ? boost::lexical_cast<uint32_t>(id_) : inst->id();
  }

  const char *get_name(arena_allocator &alloc) const {
    return string_dup(name_, alloc);
  }
  const char *get_ns(const field_instruction *inst,
                     arena_allocator &alloc) const {
    return ns_ ? string_dup(ns_, alloc) : inst->ns();
  }
};
}
}
