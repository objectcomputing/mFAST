// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "field_instructions.h"
#include "aggregate_ref.h"
#include "field_ref.h"
#include <vector>
#include <boost/iterator/iterator_facade.hpp>

namespace mfast {
struct field_view_info {
  static const uint64_t CONTINUE_BIT = (1LL << 63);

  uint64_t prop;
  typedef std::make_signed<std::size_t>::type nest_index_t;
  const nest_index_t *nest_indices;

  bool cont() const { return (prop & CONTINUE_BIT) != 0; }
  uint64_t prefix_diff() const { return prop & ~CONTINUE_BIT; }
};

struct aggregate_view_info {
  const char *name_;
  const group_field_instruction *instruction_;
  mfast::array_view<const field_view_info> data_;
  unsigned max_depth_;
  aggregate_view_info() {}
  aggregate_view_info(const char *name, const group_field_instruction *inst,
                      mfast::array_view<const field_view_info> data,
                      unsigned max_depth)
      : name_(name), instruction_(inst), data_(std::move(data)),
        max_depth_(max_depth) {}
};

class view_iterator;

class MFAST_EXPORT view_iterator_helper {
public:
  static bool access_field(view_iterator *itr);
  static void increment(view_iterator *itr);
};

class view_iterator
    : public boost::iterator_facade<view_iterator, field_cref,
                                    boost::forward_traversal_tag, field_cref>

{
public:
  view_iterator(aggregate_cref ref, const field_view_info *view_info,
                std::size_t max_depth)
      : view_info_(view_info) {
    non_leaves_.reserve(max_depth);
    non_leaves_.push_back(ref.begin());
    access_field();
  }

  view_iterator(const field_view_info *view_info) : view_info_(view_info) {}

private:
  friend class boost::iterator_core_access;
  friend class view_iterator_helper;

  field_cref dereference() const { return leaf_; }
  bool access_field() { return view_iterator_helper::access_field(this); }
  void increment() { return view_iterator_helper::increment(this); }
  bool equal(const view_iterator &other) const {
    return this->view_info_ == other.view_info_;
  }
  const field_view_info *view_info_;
  typedef std::vector<aggregate_cref::iterator> non_leave_stack_t;
  non_leave_stack_t non_leaves_;
  field_cref leaf_;
};

} /* mfast */
