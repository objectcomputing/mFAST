#include "third_pass_visitor.h"
#include "aggregate_inserter.h"
#include "sequence_inserter.h"
namespace mfast {
namespace SQLite3 {
void third_pass_visitor::visit(const aggregate_cref &ref, int rowid) {

  primary_key_.refers_to(core_.primary_key_of(ref));

  if (primary_key_.absent()) {
    // the table has no primary key, use the last rowid instead
    static const int64_field_instruction rowid_instruction(
        operator_none, presence_optional, 0, "", "", nullptr,
        int_value_storage<int64_t>());

    field_cref rowid_cref(&this->key_storage_, &rowid_instruction);
    this->key_storage_.set(rowid);
    this->key_storage_.present(true);

    primary_key_.refers_to(rowid_cref);
  }

  for (std::size_t i = 0; i < ref.num_fields(); ++i) {
    field_cref field(ref[i]);
    if (!core_.to_skip(field) && field.present()) {
      apply_accessor(*this, field);
    }
  }
}

void third_pass_visitor::visit(const group_cref &ref, int) {
  if (ref.instruction()->ref_instruction() != nullptr) {
    // output the content of ref to another table represent ref.
    // the first column of the new table is the current primary_key
    aggregate_inserter inserter(core_);
    inserter.insert(ref, primary_key_);
  }
}

void third_pass_visitor::visit(const sequence_cref &ref, int) {
  const sequence_field_instruction *inst = ref.instruction();
  const template_instruction *ref_instruction =
      dynamic_cast<const template_instruction *>(inst->ref_instruction());

  const template_instruction *element_instruction =
      dynamic_cast<const template_instruction *>(inst->element_instruction());

  bool is_ordered = core_.is_ordered(inst);

  if (ref_instruction) {

    if (element_instruction) {
      // output all the data in the sequence and then collect all the primary
      // id/rowid as an array
      for (std::size_t i = 0; i < ref.size(); ++i) {
        aggregate_inserter element_inserter(core_);

        aggregate_cref element = ref[i];
        const template_info &seq_info = core_.info(ref_instruction);
        core_.bind(seq_info, 1, primary_key_);
        core_.bind(seq_info, 2, element_inserter.insert(element, field_cref()));
        if (is_ordered) {
          core_.bind(seq_info, 3, i);
        }

        core_.exec_stmt(seq_info);
      }
    } else {
      const template_info &seq_info = core_.info(ref_instruction);
      core_.bind(seq_info, 1, primary_key_);
      sequence_inserter seq_inserter(core_, seq_info, 2, is_ordered);
      seq_inserter.visit(ref, 0);
    }
  }
}

} /* SQLite3 */

} /* mfast */
