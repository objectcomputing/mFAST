#include "first_pass_visitor.h"
#include "field_instruction_visitor_ex.h"

namespace mfast {
namespace SQLite3 {
class null_binder_visitor : public field_instruction_visitor_ex {
public:
  const converter_core &core_;
  const template_info &info_;
  unsigned &current_column_;

  null_binder_visitor(const converter_core &core, const template_info &info,
                      unsigned &current_column)
      : core_(core), info_(info), current_column_(current_column) {}

  void visit(const integer_field_instruction_base *, void *) {
    core_.bind_null(info_, current_column_++);
  }

  void visit(const ascii_field_instruction *, void *) {
    core_.bind_null(info_, current_column_++);
  }

  void visit(const vector_field_instruction_base *inst, void *) {
    BOOST_THROW_EXCEPTION(
        error() << reason_info("deeply nested int_vector is not supported")
                << field_name_info(inst->name()));
  }

  void visit(const templateref_instruction *, void *) {
    BOOST_THROW_EXCEPTION(
        std::runtime_error("dynamic templateRef is not supported"));
  }

  void traverse_aggregate(const group_field_instruction *inst) {
    for (unsigned i = 0; i < inst->subinstructions().size(); ++i) {
      const field_instruction *subinst = inst->subinstruction(i);
      if (!core_.to_skip(subinst)) {
        subinst->accept(*this, nullptr);
      }
    }
  }

  void visit(const group_field_instruction *inst, void *) {
    if (inst->ref_instruction() == nullptr) {
      traverse_aggregate(inst);
    }
  }

  void visit(const sequence_field_instruction *, void *) {
    BOOST_THROW_EXCEPTION(
        std::runtime_error("deeply nested sequence is not supported"));
  }

  void visit(const template_instruction *inst, void *) {
    traverse_aggregate(inst);
  }

  void bind(const sequence_field_instruction *inst) {
    traverse_aggregate(inst);
  }
};

first_pass_visitor::first_pass_visitor(const converter_core &core,
                                       const group_field_instruction *inst,
                                       const field_cref &foreign_key)
    : value_binder(core, core.info(inst), 1), nested_group_pass_needed_(false),
      nested_sequence_pass_needed_(false) {
  if (foreign_key.present())
    this->bind(foreign_key);
}

void first_pass_visitor::visit(const sequence_cref &ref, int) {
  const sequence_field_instruction *inst = ref.instruction();

  const template_instruction *ref_instruction =
      dynamic_cast<const template_instruction *>(inst->ref_instruction());

  const template_instruction *element_instruction =
      dynamic_cast<const template_instruction *>(inst->element_instruction());

  if (ref_instruction == nullptr && element_instruction == nullptr) {
    null_binder_visitor binder(core_, info_, current_column_);
    binder.bind(inst);
  } else {
    nested_group_pass_needed_ = true;
  }
  nested_sequence_pass_needed_ = (ref_instruction == nullptr);
}

void first_pass_visitor::visit(const aggregate_cref &ref, int) {

  this->traverse(ref);
  if (!nested_sequence_pass_needed_) {
    // in this case we have finished binding all value,
    // we should be able to execute it
    rowid_ = core_.exec_stmt(info_);
  }
}

void first_pass_visitor::traverse(const aggregate_cref &ref) {
  for (std::size_t i = 0; i < ref.num_fields(); ++i) {
    field_cref field(ref[i]);
    if (!core_.to_skip(field)) {
      if (field.absent()) {
        this->bind_null();
      } else {
        apply_accessor(*this, field);
      }
    }
  }
}

} /* SQLite3 */

} /* mfast */
