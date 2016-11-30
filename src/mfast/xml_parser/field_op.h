// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include <cstring>
#include <map>
#include <boost/lexical_cast.hpp>
#include "../arena_allocator.h"
#include "../instructions/field_instruction.h"
#include "xml_util.h"
#include "../exceptions.h"

namespace mfast {
class decimal_field_instruction;
template <typename T> class int_field_instruction;

typedef int_field_instruction<int32_t> int32_field_instruction;
typedef int_field_instruction<uint32_t> uint32_field_instruction;
typedef int_field_instruction<int64_t> int64_field_instruction;
typedef int_field_instruction<uint64_t> uint64_field_instruction;

class ascii_field_instruction;
class enum_field_instruction;
class byte_vector_field_instruction;

namespace xml_parser {
using namespace tinyxml2;

struct tag_reason;
typedef boost::error_info<tag_reason, std::string> reason_info;

class field_op {
public:
  operator_enum_t op_;
  const op_context_t *context_;
  value_storage initial_value_;
  arena_allocator *alloc_;

  field_op() : op_(operator_none), context_(nullptr), alloc_(nullptr) {}
  template <typename Instruction>
  field_op(const Instruction *inst, const XMLElement *element,
           arena_allocator &alloc)
      : op_(inst->field_operator()), context_(inst->op_context()),
        initial_value_(inst->initial_value()), alloc_(&alloc) {
    if (element) {
      const XMLElement *field_op_element = find_field_op_element(*element);
      if (field_op_element) {
        parse_field_op(*field_op_element, alloc);
        const char *init_value_str =
            get_optional_attr(*field_op_element, "value", nullptr);
        if (init_value_str)
          set_init_value(init_value_str, inst);
      }
    }
  }

  field_op(const decimal_field_instruction *inst, const XMLElement *element,
           arena_allocator &alloc);

  arena_allocator &alloc() { return *alloc_; }

  static const XMLElement *find_field_op_element(const XMLElement &element);

private:
  void set_init_value(const char *init_value_str,
                      const int32_field_instruction *) {
    if (init_value_str) {
      initial_value_.present(true);
      initial_value_.set(boost::lexical_cast<int32_t>(init_value_str));
    }
  }

  void set_init_value(const char *init_value_str,
                      const uint32_field_instruction *) {
    if (init_value_str) {
      initial_value_.present(true);
      initial_value_.set(boost::lexical_cast<uint32_t>(init_value_str));
    }
  }

  void set_init_value(const char *init_value_str,
                      const int64_field_instruction *) {
    if (init_value_str) {
      initial_value_.present(true);
      initial_value_.set(boost::lexical_cast<int64_t>(init_value_str));
    }
  }

  void set_init_value(const char *init_value_str,
                      const uint64_field_instruction *) {
    if (init_value_str) {
      initial_value_.present(true);
      initial_value_.set(boost::lexical_cast<uint64_t>(init_value_str));
    }
  }

  void set_init_value(const char *init_value_str,
                      const ascii_field_instruction *) {
    if (init_value_str) {
      initial_value_ =
          string_value_storage(string_dup(init_value_str, alloc())).storage_;
    }
  }

  void set_init_value(const char *init_value_str,
                      const enum_field_instruction *) {
    // we cannot really decide the exact enum value at this point,
    // just ignore it and process it later.
    if (init_value_str) {
      initial_value_ = string_value_storage(init_value_str).storage_;
      // we use the defined_bit to differentiate whether the content is
      // converted to int value
      // or not
      initial_value_.of_array.defined_bit_ = 0;
    }
  }

  void set_init_value(const char *init_value_str,
                      const byte_vector_field_instruction *);


  void parse_field_op(const XMLElement &field_op_element,
                      arena_allocator &alloc);
};
}
}
