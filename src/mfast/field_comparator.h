// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once
#include <utility>
#include "field_visitor.h"
namespace mfast {
namespace detail {
class field_comparator {
private:
  field_cref rhs_;

public:
  field_comparator(const field_cref &rhs) : rhs_(rhs) {}
  template <typename SimpleType> bool visit(const SimpleType &lhs) const {
    SimpleType rhs = static_cast<SimpleType>(rhs_);
    return lhs == rhs;
  }

  static bool compare(const aggregate_cref lhs, const aggregate_cref rhs) {
    if (lhs.num_fields() == rhs.num_fields()) {
      for (std::size_t i = 0; i < lhs.num_fields(); ++i) {
        if (lhs.instruction()->field_type() !=
                rhs.instruction()->field_type() ||
            lhs.present() != rhs.present())
          return false;

        if (lhs.present()) {
          field_comparator comparator(rhs[i]);
          if (!apply_accessor(comparator, lhs[i]))
            return false;
        }
      }
      return true;
    }
    return false;
  }

  bool visit(const aggregate_cref &lhs, int) const {
    return compare(lhs, static_cast<aggregate_cref>(rhs_));
  }

  bool visit(const sequence_cref &lhs, int) const {
    sequence_cref rhs = static_cast<sequence_cref>(rhs_);
    if (lhs.num_fields() == rhs.num_fields() && lhs.size() == rhs.size()) {
      for (std::size_t i = 0; i < lhs.size(); ++i) {
        if (!compare(lhs[i], rhs[i]))
          return false;
      }
      return true;
    }
    return false;
  }
};
}

inline bool operator==(const aggregate_cref &lhs, const aggregate_cref &rhs) {
  return detail::field_comparator::compare(lhs, rhs);
}

inline bool operator==(const field_cref &lhs, const field_cref &rhs) {
  if (lhs.instruction()->field_type() != rhs.instruction()->field_type())
    return false;

  detail::field_comparator comparator(rhs);
  return apply_accessor(comparator, lhs);
}

using namespace std::rel_ops;
}
