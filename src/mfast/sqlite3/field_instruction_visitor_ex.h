#pragma once

#include <mfast/field_instructions.h>

namespace mfast {
class field_instruction_visitor_ex : public field_instruction_visitor {
public:
  // #pragma clang diagnostic push
  // #pragma clang diagnostic ignored "-Woverloaded-virtual"
  virtual void visit(const integer_field_instruction_base *, void *) = 0;

  virtual void visit(const int32_field_instruction *, void *);
  virtual void visit(const uint32_field_instruction *, void *);
  virtual void visit(const int64_field_instruction *, void *);
  virtual void visit(const uint64_field_instruction *, void *);
  virtual void visit(const decimal_field_instruction *, void *);
  virtual void visit(const enum_field_instruction *inst, void *data);

  virtual void visit(const ascii_field_instruction *, void *) = 0;
  virtual void visit(const unicode_field_instruction *, void *);
  virtual void visit(const byte_vector_field_instruction *, void *);

  virtual void visit(const vector_field_instruction_base *, void *) = 0;
  virtual void visit(const int32_vector_field_instruction *, void *);
  virtual void visit(const uint32_vector_field_instruction *, void *);
  virtual void visit(const int64_vector_field_instruction *, void *);
  virtual void visit(const uint64_vector_field_instruction *, void *);

  virtual void visit(const group_field_instruction *, void *) = 0;
  virtual void visit(const sequence_field_instruction *, void *) = 0;
  virtual void visit(const template_instruction *, void *) = 0;
  virtual void visit(const templateref_instruction *, void *) = 0;
  // #pragma clang diagnostic pop
};

} /* mfast */
