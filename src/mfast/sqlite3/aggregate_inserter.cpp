#include "aggregate_inserter.h"
#include "first_pass_visitor.h"
#include "second_pass_visitor.h"
#include "third_pass_visitor.h"

namespace mfast {
namespace SQLite3 {

aggregate_inserter::aggregate_inserter(const converter_core &core)
    : core_(core) {}

field_cref aggregate_inserter::insert(const aggregate_cref &ref,
                                      const field_cref &foreign_key) {
  int rowid = 0;
  first_pass_visitor first_vistor(core_, ref.instruction(), foreign_key);
  first_vistor.visit(ref, 0);
  rowid = first_vistor.inserted_id();
  if (first_vistor.nested_sequence_pass_needed()) {
    second_pass_visitor second_vistor(core_, ref.instruction(),
                                      foreign_key.absent() ? 1 : 2);
    second_vistor.visit(ref, 0);
  }

  if (first_vistor.nested_group_pass_needed()) {
    third_pass_visitor third_vistor(core_);
    third_vistor.visit(ref, rowid);
  }

  field_cref primary_key = core_.primary_key_of(ref);

  if (primary_key.present())
    return primary_key;
  else {
    static const int64_field_instruction rowid_instruction(
        operator_none, presence_optional, 0, "", "", nullptr,
        int_value_storage<int64_t>());
    field_cref rowid_cref(&this->storage_, &rowid_instruction);
    this->storage_.set(rowid);
    this->storage_.present(true);
    return rowid_cref;
  }
}
} /* SQLite3 */

} /* mfast */
