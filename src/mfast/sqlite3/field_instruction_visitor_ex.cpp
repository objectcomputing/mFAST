#include "field_instruction_visitor_ex.h"

namespace mfast {
void field_instruction_visitor_ex::visit(const int32_field_instruction *inst,
                                         void *data) {
  this->visit(static_cast<const integer_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(const uint32_field_instruction *inst,
                                         void *data) {
  this->visit(static_cast<const integer_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(const int64_field_instruction *inst,
                                         void *data) {
  this->visit(static_cast<const integer_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(const uint64_field_instruction *inst,
                                         void *data) {
  this->visit(static_cast<const integer_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(const decimal_field_instruction *inst,
                                         void *data) {
  this->visit(static_cast<const integer_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(const enum_field_instruction *inst,
                                         void *data) {
  this->visit(static_cast<const integer_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(const unicode_field_instruction *inst,
                                         void *data) {
  this->visit(static_cast<const ascii_field_instruction *>(inst), data);
}

void field_instruction_visitor_ex::visit(
    const byte_vector_field_instruction *inst, void *data) {
  this->visit(static_cast<const ascii_field_instruction *>(inst), data);
}

void field_instruction_visitor_ex::visit(
    const int32_vector_field_instruction *inst, void *data) {
  this->visit(static_cast<const vector_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(
    const uint32_vector_field_instruction *inst, void *data) {
  this->visit(static_cast<const vector_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(
    const int64_vector_field_instruction *inst, void *data) {
  this->visit(static_cast<const vector_field_instruction_base *>(inst), data);
}

void field_instruction_visitor_ex::visit(
    const uint64_vector_field_instruction *inst, void *data) {
  this->visit(static_cast<const vector_field_instruction_base *>(inst), data);
}

} /* mfast */