// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <map>
#include <string>
#include "dynamic_templates_description.h"

namespace mfast {
namespace xml_parser {
struct template_registry_impl {
  typedef std::map<std::string, const field_instruction *> map_type;
  map_type types_;
  arena_allocator alloc_;

  std::string get_key(const char *ns, const char *name) const {
    return std::string(ns) + "||" + name;
  }

  const field_instruction *find(const char *ns, const char *name) const {
    auto itr = types_.find(get_key(ns, name));
    if (itr != types_.end()) {
      return itr->second;
    }
    return nullptr;
  }

  void add(const char *ns, const field_instruction *inst) {
    types_[get_key(ns, inst->name())] = inst;
  }
};
}
}
