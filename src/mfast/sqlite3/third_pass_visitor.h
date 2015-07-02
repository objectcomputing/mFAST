#pragma once

#include "converter_core.h"

namespace mfast {
namespace SQLite3 {
class third_pass_visitor {
  const converter_core &core_;
  field_cref primary_key_;
  value_storage key_storage_;

public:
  third_pass_visitor(const converter_core &core) : core_(core) {}
  template <typename CREF> void visit(const CREF &) {}

  void visit(const group_cref &ref, int);
  void visit(const sequence_cref &ref, int);
  void visit(const aggregate_cref &ref, int rowid);
};

} /* SQLite3 */

} /* mfast */
