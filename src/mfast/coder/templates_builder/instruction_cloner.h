#ifndef INSTRUCTION_CLONER_H_VXZQKX5U
#define INSTRUCTION_CLONER_H_VXZQKX5U

#include "field_builder_base.h"

namespace mfast
{
namespace coder
{
class instruction_cloner
  : public field_instruction_visitor
{
public:

  instruction_cloner(field_builder_base& list,
                     arena_allocator&    alloc)
    : list_(list)
    , alloc_(alloc)
  {
  }

private:
  field_builder_base& list_;
  arena_allocator&    alloc_;

  virtual void visit(const int32_field_instruction* inst, void*)
  {
    int32_field_instruction* new_inst = new (alloc_) int32_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const uint32_field_instruction* inst, void*)
  {
    uint32_field_instruction* new_inst = new (alloc_) uint32_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const int64_field_instruction* inst, void*)
  {
    int64_field_instruction* new_inst = new (alloc_) int64_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const uint64_field_instruction* inst, void*)
  {
    uint64_field_instruction* new_inst = new (alloc_) uint64_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const decimal_field_instruction* inst, void*)
  {
    decimal_field_instruction* new_inst = new (alloc_) decimal_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const ascii_field_instruction* inst, void*)
  {
    ascii_field_instruction* new_inst = new (alloc_) ascii_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const unicode_field_instruction* inst, void*)
  {
    unicode_field_instruction* new_inst = new (alloc_) unicode_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const byte_vector_field_instruction* inst, void*)
  {
    byte_vector_field_instruction* new_inst = new (alloc_) byte_vector_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const int32_vector_field_instruction* inst, void*)
  {
    int32_vector_field_instruction* new_inst = new (alloc_) int32_vector_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const uint32_vector_field_instruction* inst, void*)
  {
    uint32_vector_field_instruction* new_inst = new (alloc_) uint32_vector_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const int64_vector_field_instruction* inst, void*)
  {
    int64_vector_field_instruction* new_inst = new (alloc_) int64_vector_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const uint64_vector_field_instruction* inst, void*)
  {
    uint64_vector_field_instruction* new_inst = new (alloc_) uint64_vector_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const group_field_instruction* inst, void*)
  {
    group_field_instruction* new_inst = new (alloc_) group_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const sequence_field_instruction* inst, void*)
  {
    sequence_field_instruction* new_inst = new (alloc_) sequence_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const templateref_instruction* inst, void*)
  {
    templateref_instruction* new_inst = new (alloc_) templateref_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

  virtual void visit(const template_instruction* inst, void*)
  {
    for (std::size_t i = 0; i < inst->subinstructions_count(); ++i) {
      inst->subinstruction(i)->accept(*this, 0);
    }
  }

  virtual void visit(const enum_field_instruction* inst, void*)
  {
    enum_field_instruction* new_inst = new (alloc_) enum_field_instruction(*inst);
    new_inst->field_index(list_.num_instructions());
    list_.add_instruction(new_inst);
  }

};


}   /* coder */

} /* mfast */

#endif /* end of include guard: INSTRUCTION_CLONER_H_VXZQKX5U */
