#include "second_pass_visitor.h"
#include "aggregate_inserter.h"
#include "sequence_inserter.h"

namespace mfast {
namespace SQLite3 {
void second_pass_visitor::visit(const sequence_cref &ref, int) {
  const sequence_field_instruction *inst = ref.instruction();
  const template_instruction *ref_instruction =
      dynamic_cast<const template_instruction *>(inst->ref_instruction());

  const template_instruction *element_instruction =
      dynamic_cast<const template_instruction *>(inst->element_instruction());

  bool is_ordered = core_.is_ordered(inst);

  if (ref_instruction == nullptr) {
    if (ref.size()) {

      if (element_instruction) {
        // output all the data in the sequence and then collect all the primary
        // id as an array
        for (std::size_t i = 0; i < ref.size(); ++i) {
          aggregate_inserter element_inserter(core_);

          field_cref key_ref = element_inserter.insert(ref[i], field_cref());
          core_.bind(this->info_, column_index_, key_ref);
          if (is_ordered)
            core_.bind(this->info_, column_index_ + 1, i);
          core_.exec_stmt(this->info_);
        }
      } else { // element_instruction == 0
        // bind the nested values in the current statement
        sequence_inserter seq_inserter(core_, this->info_, column_index_,
                                       is_ordered);
        seq_inserter.visit(ref, 0);
      }
    } else {
      // there is no element in the sequence, the columns related to the
      // sequence has been binded to null in the
      // first pass, just execute the statement
      core_.exec_stmt(this->info_);
    }
  }
}

} /* SQLite3 */

} /* mfast */
