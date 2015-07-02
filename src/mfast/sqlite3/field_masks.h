#pragma once

#include <mfast.h>
#include <sqlite3.h>
namespace mfast {
namespace SQLite3 {
struct field_masks {
  field_masks(unsigned skip = 0, unsigned primary_key = 0, unsigned ordered = 0)
      : skip_mask(skip), primary_key_mask(primary_key), ordered_mask(ordered) {}

  unsigned skip_mask;
  unsigned primary_key_mask;
  unsigned ordered_mask;

  bool is_primary_key(const field_instruction *inst) const {
    return inst->tag().to_uint64() & this->primary_key_mask;
  }

  bool is_primary_key(const field_cref &ref) const {
    return is_primary_key(ref.instruction());
  }
  bool to_skip(const field_instruction *inst) const {
    return inst->tag().to_uint64() & this->skip_mask;
  }

  bool to_skip(const field_cref &ref) const {
    return to_skip(ref.instruction());
  }
  bool has_constant_exponent(const decimal_field_instruction *inst) const {
    return (inst->field_type() == field_type_exponent &&
            inst->field_operator() == operator_constant);
  }

  bool has_constant_exponent(const decimal_cref &ref) const {
    return has_constant_exponent(ref.instruction());
  }

  bool is_ordered(const sequence_field_instruction *inst) const {
    return inst->tag().to_uint64() & this->ordered_mask;
  }
};

} /* SQLite3 */
} /* mfast */
