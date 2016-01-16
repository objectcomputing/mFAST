#pragma once

#include "converter_core.h"

namespace mfast {
namespace SQLite3 {
class second_pass_visitor {
  const converter_core &core_;
  const template_info &info_;
  unsigned column_index_;

public:
  second_pass_visitor(const converter_core &core,
                      const group_field_instruction *inst,
                      unsigned start_column = 1)
      : core_(core), info_(core.info(inst)), column_index_(start_column) {}

  template <typename CREF> void visit(const CREF &) { ++column_index_; }

  template <typename T> void visit(const int_vector_cref<T> &) {
    // TODO: implemnet later
  }

  void visit(const group_cref &ref, int) {
    if (ref.instruction()->ref_instruction() == nullptr) {
      // keep traversing
      traverse(ref);
    } else {
      //  do it in nested_group_pass
    }
  }

  void visit(const aggregate_cref &ref, int) { traverse(ref); }
  void visit(const sequence_cref &ref, int);

private:
  void traverse(const aggregate_cref &ref) {
    for (std::size_t i = 0; i < ref.num_fields(); ++i) {
      field_cref field(ref[i]);
      if (!core_.to_skip(field)) {
        apply_accessor(*this, field);
      }
    }
  }
};

} /* SQLite3 */
} /* mfast */
