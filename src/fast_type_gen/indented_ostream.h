// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <ostream>
#include <iomanip>
#include <sstream>
struct indent_t {};

class indented_stringstream {
public:
  indented_stringstream() : indent_level_(0) {}
  void inc_indent(std::size_t level = 1) { indent_level_ += level; }
  void dec_indent(std::size_t level = 1) { indent_level_ -= level; }
  void reset_indent(std::size_t level = 0) { indent_level_ = level; }
  std::size_t indent_level() const { return indent_level_; }
  indented_stringstream &operator<<(indent_t) {
    if (indent_level() > 0)
      os_ << std::setw(indent_level() * 2) << ' ';
    return *this;
  }

  template <typename T> indented_stringstream &operator<<(const T &t) {
    os_ << t;
    return *this;
  }

  std::string str() const { return os_.str(); }
  void str(const char *s) { os_.str(s); }
  void clear() { os_.clear(); }

private:
  std::size_t indent_level_;
  std::stringstream os_;
};
