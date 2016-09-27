#pragma once

#include "../mfast_coder_export.h"
#include "../../sequence_ref.h"
#include "../../nested_message_ref.h"
#include "../../malloc_allocator.h"
#include "../common/template_repo.h"
#include "../common/exceptions.h"
#include "../encoder/fast_ostream.h"
#include "../encoder/resizable_fast_ostreambuf.h"
#include "../encoder/encoder_presence_map.h"
#include "../../ext_ref.h"
#include "fast_ostream_inserter.h"
#include <tuple>

namespace mfast {
namespace coder {
template <typename T>
inline bool equivalent(const int_cref<T> &v, const value_storage &prev) {
  return v.absent() == prev.is_empty() && v.value() == prev.get<T>();
}

inline bool equivalent(const exponent_cref &v, const value_storage &prev) {
  return v.absent() == prev.is_empty() &&
         v.value() == prev.of_decimal.exponent_;
}

inline bool equivalent(const decimal_cref &v, const value_storage &prev) {
  return v.absent() == prev.is_empty() &&
         v.mantissa() == prev.of_decimal.mantissa_ &&
         v.exponent() == prev.of_decimal.exponent_;
}

template <typename T>
inline bool equivalent(const vector_cref<T> &v, const value_storage &prev) {
  return v.size() == prev.of_array.len_ - 1 &&
         memcmp(v.data(), prev.of_array.content_, v.size() * sizeof(T)) == 0;
}

struct fast_encoder_core;

struct MFAST_CODER_EXPORT fast_encoder_core : mfast::detail::codec_helper {
  fast_encoder_core(allocator *alloc);

  void allow_overlong_pmap_i(bool v);

  /// encoder initialization functions
  template <typename Message> void register_message();

  template <typename... Desc> void init(Desc... rest) {
    static_assert(sizeof...(rest) > 0, "There must exist at least one "
                                       "parameter of type const "
                                       "templates_description*");
    repo_.build(rest...);
    active_message_info_ = repo_.unique_entry();
  }

  /// message encode functions
  void encode_segment(const message_cref cref, bool force_reset);

  template <typename Message> void encode_message(const message_cref &cref);

  std::size_t encode_i(const message_cref &message, char *buffer,
                       std::size_t buffer_size, bool force_reset);

  void encode_i(const message_cref &message, std::vector<char> &buffer,
                bool force_reset);

  /// vistation functions for mFAST data structures
  template <typename T> void visit(const T &ext_ref);
  void visit(const nested_message_cref &cref);

  /// FAST encoding rules
  template <typename T, typename TypeCategory>
  void encode_field(const T &ext_ref, TypeCategory);

  template <typename T>
  void encode_field(const T &ext_ref, split_decimal_type_tag);

  template <typename T>
  void encode_field(const T &ext_ref, int_vector_type_tag);

  template <typename T> void encode_field(const T &ext_ref, group_type_tag);

  template <typename T> void encode_field(const T &ext_ref, sequence_type_tag);

  template <typename T, typename TypeCategory>
  void encode_field(const T &ext_ref, none_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void encode_field(const T &ext_ref, constant_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void encode_field(const T &ext_ref, copy_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void encode_field(const T &ext_ref, increment_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void encode_field(const T &ext_ref, default_operator_tag, TypeCategory);

  template <typename T>
  void encode_field(const T &ext_ref, delta_operator_tag, integer_type_tag);

  template <typename T>
  void encode_field(const T &ext_ref, delta_operator_tag, string_type_tag);

  template <typename T>
  void encode_field(const T &ext_ref, delta_operator_tag, decimal_type_tag);

  template <typename T>
  void encode_field(const T &ext_ref, tail_operator_tag, string_type_tag);

  typedef void (fast_encoder_core::*message_encode_function_t)(
      const message_cref &);
  typedef std::tuple<template_instruction *, message_encode_function_t>
      info_entry;

  struct info_entry_converter {
    typedef info_entry repo_mapped_type;

    template <typename Message>
    info_entry to_repo_entry(template_instruction *inst, Message *) {
      return info_entry(inst, &fast_encoder_core::encode_message<Message>);
    }

    template_instruction *to_instruction(const repo_mapped_type &entry) {
      return std::get<0>(entry);
    }
  };

  /// internal states

  template_repo<info_entry_converter> repo_;
  fast_ostream strm_;
  info_entry *active_message_info_;
  encoder_presence_map *current_;
};

template <typename T> class encoder_pmap_saver {
  encoder_presence_map pmap_;
  encoder_presence_map *prev_pmap_;
  fast_encoder_core *core_;

public:
  encoder_pmap_saver(fast_encoder_core *core)
      : prev_pmap_(core->current_), core_(core) {
    core_->current_ = &pmap_;
    this->pmap_.init(&core_->strm_, T::value);
  }

  ~encoder_pmap_saver() {
    this->pmap_.commit();
    core_->current_ = this->prev_pmap_;
  }
};

template <> class encoder_pmap_saver<pmap_segment_size_zero> {
public:
  encoder_pmap_saver(fast_encoder_core *) {}
  ~encoder_pmap_saver() {}
};

inline fast_encoder_core::fast_encoder_core(allocator *alloc)
    : repo_(alloc), strm_(alloc), active_message_info_(nullptr),
      current_(nullptr) {}

template <typename Message>
void fast_encoder_core::encode_message(const message_cref &cref) {
  typename Message::cref_type ref(cref);
  ref.accept(*this);
}

inline std::size_t fast_encoder_core::encode_i(const message_cref &message,
                                               char *buffer,
                                               std::size_t buffer_size,
                                               bool force_reset) {
  assert(buffer_size > 0);

  fast_ostreambuf sb(buffer, buffer_size);
  this->strm_.rdbuf(&sb);
  this->encode_segment(message, force_reset);
  return sb.length();
}

inline void fast_encoder_core::encode_i(const message_cref &message,
                                        std::vector<char> &buffer,
                                        bool force_reset) {
  resizable_fast_ostreambuf sb(buffer);
  this->strm_.rdbuf(&sb);
  this->encode_segment(message, force_reset);
  buffer.resize(sb.length());
}

inline void fast_encoder_core::allow_overlong_pmap_i(bool v) {
  this->strm_.allow_overlong_pmap(v);
}
template <typename T> inline void fast_encoder_core::visit(const T &ext_ref) {
  typedef typename T::type_category type_category;
  this->encode_field(ext_ref, type_category());
}

inline void fast_encoder_core::visit(const nested_message_cref &cref) {
  info_entry *saved_message_info = active_message_info_;
  encoder_presence_map *prev_pmap = this->current_;
  encode_segment(cref.target(), false);
  this->current_ = prev_pmap;
  active_message_info_ = saved_message_info;
}

template <typename T, typename TypeCategory>
inline void fast_encoder_core::encode_field(const T &ext_ref, TypeCategory) {
  this->encode_field(ext_ref, typename T::operator_category(), TypeCategory());
}

template <typename T>
inline void fast_encoder_core::encode_field(const T &ext_ref,
                                            split_decimal_type_tag) {
  typename T::exponent_type exponent_ref = ext_ref.get_exponent();
  this->visit(exponent_ref);
  if (exponent_ref.present()) {
    this->visit(ext_ref.get_mantissa());
  }
}

template <typename T>
inline void fast_encoder_core::encode_field(const T &ext_ref,
                                            int_vector_type_tag) {
  typename T::cref_type cref = ext_ref.get();

  this->strm_.encode(static_cast<uint32_t>(cref.size()), !cref.present(),
                     cref.optional());
  if (cref.present()) {
    for (auto elem : cref) {
      this->strm_.encode(elem, false, false_type());
    }
  }
}

template <typename T>
inline void fast_encoder_core::encode_field(const T &ext_ref, group_type_tag) {
  // If a group field is optional, it will occupy a single bit in the presence
  // map.
  // The contents of the group may appear in the stream iff the bit is set.
  if (ext_ref.optional()) {
    this->current_->set_next_bit(ext_ref.present());

    if (!ext_ref.present())
      return;
  }

  encoder_pmap_saver<typename T::pmap_segment_size_type> saver(this);
  ext_ref.get().accept(*this);
}

template <typename T>
inline void fast_encoder_core::encode_field(const T &ext_ref,
                                            sequence_type_tag) {
  value_storage storage;

  typename T::length_type length = ext_ref.get_length(storage);
  this->visit(length);
  std::size_t sz = length.get().value();
  for (std::size_t i = 0; i < sz; ++i) {
    this->visit(ext_ref[i]);
  }
}

template <typename T, typename TypeCategory>
void fast_encoder_core::encode_field(const T &ext_ref, none_operator_tag,
                                     TypeCategory) {
  strm_ << ext_ref;

  // Fast Specification 1.1, page 22
  //
  // If a field is mandatory and has no field operator, it will not occupy any
  // bit in the presence map and its value must always appear in the stream.
  //
  // If a field is optional and has no field operator, it is encoded with a
  // nullable representation and the NULL is used to represent absence of a
  // value. It will not occupy any bits in the presence map.
  if (ext_ref.previous_value_shared())
    strm_.save_previous_value(ext_ref.get());
}

template <typename T, typename TypeCategory>
void fast_encoder_core::encode_field(const T &ext_ref, constant_operator_tag,
                                     TypeCategory) {
  typename T::cref_type cref = ext_ref.get();
  encoder_presence_map &pmap = *current_;

  if (ext_ref.optional()) {
    // A field will not occupy any bit in the presence map if it is mandatory
    // and has the
    // constant operator.
    // An optional field with the constant operator will occupy a single bit. If
    // the bit is set,
    // the value
    // is the initial value in the instruction context. If the bit is not set,
    // the value is
    // considered absent.
    pmap.set_next_bit(cref.present());
  }

  if (ext_ref.previous_value_shared())
    strm_.save_previous_value(cref);
}

template <typename T, typename TypeCategory>
void fast_encoder_core::encode_field(const T &ext_ref, copy_operator_tag,
                                     TypeCategory) {
  encoder_presence_map &pmap = *current_;
  typename T::cref_type cref = ext_ref.get();

  value_storage previous = previous_value_of(cref);
  strm_.save_previous_value(cref);

  if (!previous.is_defined()) {
    // if the previous value is undefined – the value of the field is the
    // initial value
    // that also becomes the new previous value.
    // If the field has optional presence and no initial value, the field is
    // considered
    // absent and the state of the previous value is changed to empty.
    if (cref.is_initial_value()) {
      pmap.set_next_bit(false);
      return;
    }
  } else if (previous.is_empty()) {
    // if the previous value is empty – the value of the field is empty.
    // If the field is optional the value is considered absent.
    if (!ext_ref.present()) {
      pmap.set_next_bit(false);
      return;
    } else if (!ext_ref.optional()) {
      // It is a dynamic error [ERR D6] if the field is mandatory.
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));

      // We need to handle this case because the previous value may have been
      // modified by another instruction with the same key and that intruction
      // has optional presence.
    }
  } else if (equivalent(cref, previous)) {
    pmap.set_next_bit(false);
    return;
  }

  pmap.set_next_bit(true);
  strm_ << ext_ref;
}

template <typename T, typename TypeCategory>
void fast_encoder_core::encode_field(const T &ext_ref, increment_operator_tag,
                                     TypeCategory) {
  encoder_presence_map &pmap = *current_;
  typename T::cref_type cref = ext_ref.get();

  value_storage previous = previous_value_of(cref);
  strm_.save_previous_value(cref);

  if (!previous.is_defined()) {
    // if the previous value is undefined – the value of the field is the
    // initial value
    // that also becomes the new previous value.
    // If the field has optional presence and no initial value, the field is
    // considered
    // absent and the state of the previous value is changed to empty.
    if (cref.is_initial_value()) {
      pmap.set_next_bit(false);
      return;
    }
  } else if (previous.is_empty()) {
    // if the previous value is empty – the value of the field is empty.
    // If the field is optional the value is considered absent.
    if (!ext_ref.present()) {
      pmap.set_next_bit(false);
      return;
    } else if (!ext_ref.optional()) {
      // It is a dynamic error [ERR D6] if the field is mandatory.
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));

      // We need to handle this case because the previous value may have been
      // modified by another instruction with the same key and that intruction
      // has optional presence.
    }
  } else if (cref.value() ==
             previous.get<const typename T::cref_type::value_type>() + 1) {
    pmap.set_next_bit(false);
    return;
  }

  pmap.set_next_bit(true);
  strm_ << ext_ref;
}

template <typename T, typename TypeCategory>
void fast_encoder_core::encode_field(const T &ext_ref, default_operator_tag,
                                     TypeCategory) {
  encoder_presence_map &pmap = *current_;
  typename T::cref_type cref = ext_ref.get();

  // Mandatory integer, decimal, string and byte vector fields – one bit. If
  // set, the value
  // appears in the stream.
  // Optional integer, decimal, string and byte vector fields – one bit. If set,
  // the value appears
  // in the stream in a nullable representation.

  //  The default operator specifies that the value of a field is either present
  //  in the stream
  //  or it will be the initial value.

  // If the field has optional presence and no initial value, the field is
  // considered absent
  // when there is no value in the stream.

  if (cref.is_initial_value()) {
    pmap.set_next_bit(false);
    if (ext_ref.previous_value_shared())
      strm_.save_previous_value(cref);
    return;
  }

  pmap.set_next_bit(true);
  if (!ext_ref.present()) {
    //  A NULL indicates that the value is absent and the state of the previous
    //  value is left
    //  unchanged.
    strm_.encode_null();
  } else {
    strm_ << ext_ref;
    if (ext_ref.previous_value_shared())
      strm_.save_previous_value(cref);
  }
}

template <typename T>
void fast_encoder_core::encode_field(const T &ext_ref, delta_operator_tag,
                                     integer_type_tag) {
  typename T::cref_type cref = ext_ref.get();

  if (!ext_ref.present()) {
    //  If the field has optional presence, the delta value can be NULL. In that
    //  case the value
    //  of the field is considered absent.
    strm_.encode_null();
  } else {
    value_storage bv = delta_base_value_of(cref);
    typename T::cref_type base(&bv, nullptr);

    int64_t delta = static_cast<int64_t>(cref.value() - base.value());

    strm_.encode(delta, false, ext_ref.nullable());
    strm_.save_previous_value(cref);
  }
}

template <typename T>
void fast_encoder_core::encode_field(const T &ext_ref, delta_operator_tag,
                                     string_type_tag) {
  typename T::cref_type cref = ext_ref.get();

  if (!ext_ref.present()) {
    strm_.encode_null();
    return;
  }

  const value_storage &prev = delta_base_value_of(cref);

  typename T::cref_type prev_cref(&prev, cref.instruction());
  typedef typename T::cref_type::const_iterator const_iterator;
  typedef typename T::cref_type::const_reverse_iterator const_reverse_iterator;
  typedef typename std::iterator_traits<const_iterator>::difference_type
      difference_type;

  std::pair<const_iterator, const_iterator> common_prefix_positions =
      std::mismatch(cref.begin(), cref.end(), prev_cref.begin());

  std::pair<const_reverse_iterator, const_reverse_iterator>
      common_suffix_positions =
          std::mismatch(cref.rbegin(), cref.rend(), prev_cref.rbegin());

  int32_t substraction_len;
  const_iterator delta_iterator;
  uint32_t delta_len;

  difference_type common_prefix_delta_len =
      cref.end() - common_prefix_positions.first;
  difference_type common_suffix_delta_len =
      cref.rend() - common_suffix_positions.first;

  if (common_prefix_delta_len <= common_suffix_delta_len) {
    substraction_len =
        static_cast<int32_t>(prev_cref.end() - common_prefix_positions.second);
    delta_iterator = common_prefix_positions.first;
    delta_len = static_cast<uint32_t>(common_prefix_delta_len);
  } else {
    // Characters are removed from the front when the subtraction length is
    // negative.
    // The subtraction length uses an excess-1 encoding: if the value is
    // negative when decoding,
    // it is incremented by one to get the number of characters to subtract.
    // This makes it
    // possible
    // to encode negative zero as -1,
    substraction_len = static_cast<int32_t>(
        ~(prev_cref.rend() - common_suffix_positions.second));
    delta_iterator = cref.begin();
    delta_len = static_cast<uint32_t>(common_suffix_delta_len);
  }

  strm_.encode(substraction_len, false, ext_ref.nullable());
  strm_.encode(delta_iterator, delta_len, cref.instruction(), false_type());

  strm_.save_previous_value(cref);
}

template <typename T>
void fast_encoder_core::encode_field(const T &ext_ref, delta_operator_tag,
                                     decimal_type_tag) {
  if (ext_ref.present()) {
    decimal_cref cref = ext_ref.get();

    value_storage bv = delta_base_value_of(cref);

    value_storage delta_storage;
    delta_storage.of_decimal.exponent_ =
        cref.exponent() - bv.of_decimal.exponent_;
    delta_storage.of_decimal.mantissa_ =
        cref.mantissa() - bv.of_decimal.mantissa_;

    delta_storage.present(true);

    decimal_cref delta(&delta_storage, cref.instruction());
    strm_ << T(delta);

    strm_.save_previous_value(cref);
  } else {
    strm_.encode_null();
  }
}

template <typename T>
void fast_encoder_core::encode_field(const T &ext_ref, tail_operator_tag,
                                     string_type_tag) {
  encoder_presence_map &pmap = *current_;
  typename T::cref_type cref = ext_ref.get();

  value_storage &prev = previous_value_of(cref);

  if (equivalent(cref, tail_base_value_of(cref))) {
    pmap.set_next_bit(false);
  } else if (!ext_ref.present()) {
    if (prev.is_defined() && prev.is_empty()) {
      pmap.set_next_bit(false);
    } else {
      pmap.set_next_bit(true);
      strm_.encode_null();
    }
  } else {
    pmap.set_next_bit(true);

    uint32_t tail_len;
    typedef typename T::cref_type::const_iterator const_iterator;

    const_iterator tail_itr;

    value_storage base = tail_base_value_of(cref);
    typename T::cref_type base_cref(&base, cref.instruction());

    if (cref.size() == base_cref.size()) {
      std::pair<const_iterator, const_iterator> positions =
          std::mismatch(cref.begin(), cref.end(), base_cref.begin());

      tail_itr = positions.first;
      tail_len = static_cast<uint32_t>(cref.end() - positions.first);
    } else {
      tail_itr = cref.begin();
      tail_len = static_cast<uint32_t>(cref.size());
    }
    strm_.encode(tail_itr, tail_len, cref.instruction(), ext_ref.nullable());
  }
  strm_.save_previous_value(cref);
}

} /* coder */
} /* mfast */
