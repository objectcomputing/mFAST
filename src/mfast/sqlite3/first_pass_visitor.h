#pragma once

#include "converter_core.h"
#include "value_binder.h"

namespace mfast {
namespace SQLite3 {
class first_pass_visitor : value_binder {
  bool nested_group_pass_needed_;
  bool nested_sequence_pass_needed_;
  int rowid_;

public:
  first_pass_visitor(const converter_core &core,
                     const group_field_instruction *inst,
                     const field_cref &foreign_key);
  template <typename CREF> void visit(const CREF &ref) { bind(ref); }

  template <typename T> void visit(const int_vector_cref<T> &) {
    nested_sequence_pass_needed_ = true;
    current_column_++;
  }

  void visit(const group_cref &ref, int) {
    if (ref.instruction()->ref_instruction() == nullptr) {
      this->traverse(ref);
    } else {
      nested_group_pass_needed_ = true;
    }
  }

  void visit(const sequence_cref &ref, int);
  void visit(const aggregate_cref &ref, int);

  int inserted_id() const { return rowid_; }
  bool nested_group_pass_needed() const { return nested_group_pass_needed_; }
  bool nested_sequence_pass_needed() const {
    return nested_sequence_pass_needed_;
  }

private:
  void traverse(const aggregate_cref &ref);
};

} /* SQLite3 */

} /* mfast */
