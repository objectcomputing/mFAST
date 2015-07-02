#pragma once

#include "value_binder.h"

namespace mfast {
namespace SQLite3 {
class sequence_inserter : value_binder {
  unsigned start_column_;
  bool ordered_;

public:
  sequence_inserter(const converter_core &core, const template_info &info,
                    unsigned start_column, bool ordered)
      : value_binder(core, info, start_column), start_column_(start_column),
        ordered_(ordered) {}

  template <typename CREF> void visit(const CREF &ref) { bind(ref); }

  template <typename T> void visit(const int_vector_cref<T> &ref) {
    for (std::size_t i = 0; i < ref.size(); ++i) {
      core_.bind(info_, start_column_, ref[i]);
      if (ordered_)
        core_.bind(info_, start_column_ + 1, i);
      core_.exec_stmt(info_);
    }
  }

  void visit(const aggregate_cref &cref, int) {
    for (std::size_t i = 0; i < cref.num_fields(); ++i) {
      field_cref field(cref[i]);
      if (!core_.to_skip(field)) {
        if (field.absent()) {
          this->bind_null();
        } else {
          apply_accessor(*this, field);
        }
      }
    }
  }

  void visit(const sequence_cref &ref, int) {
    for (std::size_t i = 0; i < ref.size(); ++i) {
      current_column_ = start_column_;
      this->visit(ref[i], i);
      if (ordered_)
        core_.bind(info_, current_column_, i);
      core_.exec_stmt(info_);
    }
  }
};

} /* SQLite3 */

} /* mfast */
