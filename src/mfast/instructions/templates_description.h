// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#pragma once

#include "template_instruction.h"

namespace mfast {
namespace coder {
class templates_builder;
}
class MFAST_EXPORT templates_description {
public:
  template <unsigned SIZE>
  templates_description(const char *ns, const char *template_ns,
                        const char *dictionary,
                        const template_instruction *(&instructions)[SIZE])
      : ns_(ns), template_ns_(template_ns), dictionary_(dictionary),
        instructions_(instructions), instructions_count_(SIZE) {}

  typedef const template_instruction **iterator;

  const char *ns() const { return ns_; }
  const char *template_ns() const { return template_ns_; }
  const char *dictionary() const { return dictionary_; }
  const template_instruction *operator[](std::size_t i) const {
    return instructions_[i];
  }
  iterator begin() const { return instructions_; }
  iterator end() const { return instructions_ + size(); }
  const template_instruction *instruction_with_id(uint32_t id) const {
    for (uint32_t i = 0; i < instructions_count_; ++i) {
      if (instructions_[i]->id() == id)
        return instructions_[i];
    }
    return nullptr;
  }

  uint32_t size() const { return instructions_count_; }

protected:
  templates_description() {}
  friend class coder::templates_builder;
  const char *ns_;
  const char *template_ns_;
  const char *dictionary_;
  const template_instruction **instructions_;
  uint32_t instructions_count_;
};

} /* mfast */
