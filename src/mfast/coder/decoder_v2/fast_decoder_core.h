// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "../mfast_coder_export.h"

#include "../../sequence_ref.h"
#include "../../nested_message_ref.h"
#include "../../malloc_allocator.h"
#include "../../output.h"
#include "../../composite_type.h"
#include "../common/exceptions.h"
#include "../common/debug_stream.h"
#include "../common/template_repo.h"
#include "../common/codec_helper.h"
#include "../decoder/decoder_presence_map.h"
#include "../common/codec_helper.h"
#include "../decoder/fast_istream.h"
#include "fast_istream_extractor.h"
#include <tuple>
#include <vector>
namespace mfast {
namespace coder {
struct fast_decoder_base;
typedef void (fast_decoder_base::*message_decode_function_t)(
    const message_mref &);

struct MFAST_CODER_EXPORT fast_decoder_base : detail::codec_helper {
  fast_decoder_base(allocator *alloc);

  template <typename Message> void decode_message(const message_mref &mref);

  template <typename T> void visit(const T &ext_ref);

  virtual void visit(const nested_message_mref &mref) = 0;

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref, TypeCategory);

  template <typename T>
  void decode_field(const T &ext_ref, split_decimal_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref, int_vector_type_tag);

  template <typename T> void decode_field(const T &ext_ref, group_type_tag);

  template <typename T> void decode_field(const T &ext_ref, sequence_type_tag);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref, none_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref, constant_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref, copy_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref, increment_operator_tag, TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref, default_operator_tag, TypeCategory);

  template <typename T>
  void decode_field(const T &ext_ref, delta_operator_tag, integer_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref, delta_operator_tag, string_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref, delta_operator_tag, decimal_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref, tail_operator_tag, string_type_tag);

  fast_istream strm_;
  allocator *message_alloc_;
  bool force_reset_;
  decoder_presence_map *current_;
};

template <typename T> class decoder_pmap_saver {
  decoder_presence_map pmap_;
  decoder_presence_map *prev_pmap_;
  fast_decoder_base *base_;

public:
  decoder_pmap_saver(fast_decoder_base *base)
      : prev_pmap_(base->current_), base_(base) {
    base_->current_ = &pmap_;
    base_->strm_.decode(pmap_);
  }

  ~decoder_pmap_saver() { base_->current_ = this->prev_pmap_; }
};

template <> class decoder_pmap_saver<pmap_segment_size_zero> {
public:
  decoder_pmap_saver(fast_decoder_base *) {}
  ~decoder_pmap_saver() {}
};

template <bool MoreThanOneToken> struct token_base {
  unsigned current_token_;

  unsigned get_token() const { return current_token_; }
  void set_token(unsigned token) { current_token_ = token; }
};

template <> struct token_base<0> {
  unsigned get_token() const { return 0; }
  void set_token(unsigned) {}
};

template <unsigned NumTokens>
struct fast_decoder_core : public fast_decoder_base,
                           token_base<(NumTokens > 1)> {
  fast_decoder_core(allocator *alloc);

  template <typename... Desc> void init(Desc... rest) {
    static_assert(sizeof...(rest) > 0, "There must exist at least one "
                                       "parameter of type const "
                                       "templates_description*");

    repo_.build(rest...);
    active_message_info_ = repo_.unique_entry();
  }

  const static unsigned num_reserved_msgs = (NumTokens == 0 ? 1 : NumTokens);

  using fast_decoder_base::visit;
  virtual void visit(const nested_message_mref &mref);

  const message_mref &decode_segment(fast_istreambuf &sb);

  const message_mref &decode_stream(unsigned token, const char *&first,
                                    const char *last, bool force_reset);

  typedef std::vector<mfast::message_type> message_resources_t;

  typedef std::pair<message_resources_t::iterator,
                    coder::message_decode_function_t> info_entry_init_t;

  struct info_entry {
    mfast::message_mref messages_[num_reserved_msgs];
    coder::message_decode_function_t decode_fun_;

    info_entry() = default;
    info_entry(info_entry_init_t init_pair) : decode_fun_(init_pair.second) {
      auto itr = init_pair.first;
      for (auto &elem : messages_) {
        elem.refers_to(itr->mref());
        ++itr;
      }
    }
  };

  struct info_entry_converter {
    typedef info_entry repo_mapped_type;
    info_entry_converter(allocator *alloc) : alloc_(alloc) {}
    template_instruction *to_instruction(const info_entry &entry) const {
      return const_cast<template_instruction *>(
          entry.messages_[0].instruction());
    }

    template <typename Message>
    info_entry_init_t to_repo_entry(template_instruction *inst, Message *) {
      std::size_t index = message_resources_.size();
      for (std::size_t i = 0; i < num_reserved_msgs; ++i)
        message_resources_.emplace_back(alloc_, inst, nullptr);

      return info_entry_init_t(message_resources_.begin() + index,
                               &fast_decoder_base::decode_message<Message>);
    }

    mfast::allocator *alloc_;
    message_resources_t message_resources_;
  };

  const message_mref &active_message() {
    return active_message_info_->messages_[this->get_token()];
  }

  template_repo<info_entry_converter> repo_;
  info_entry *active_message_info_;
};

inline fast_decoder_base::fast_decoder_base(allocator *alloc)
    : strm_(nullptr), message_alloc_(alloc), force_reset_(false),
      current_(nullptr) {}

template <typename T> inline void fast_decoder_base::visit(const T &ext_ref) {
  typedef typename T::type_category type_category;
  this->decode_field(ext_ref, type_category());
}

template <typename T, typename TypeCategory>
inline void fast_decoder_base::decode_field(const T &ext_ref, TypeCategory) {
  this->decode_field(ext_ref, typename T::operator_category(), TypeCategory());
}

template <typename T>
inline void fast_decoder_base::decode_field(const T &ext_ref,
                                            split_decimal_type_tag) {
  auto exponent_ref = ext_ref.set_exponent();
  this->visit(exponent_ref);
  if (exponent_ref.present()) {
    this->visit(ext_ref.set_mantissa());
  }
}

template <typename T>
inline void fast_decoder_base::decode_field(const T &ext_ref,
                                            int_vector_type_tag) {
  auto mref = ext_ref.set();

  uint32_t length = 0;
  if (!this->strm_.decode(length, ext_ref.optional())) {
    ext_ref.omit();
    return;
  }

  mref.resize(length);
  for (uint32_t i = 0; i < length; ++i) {
    this->strm_.decode(mref[i], false_type());
  }
}

template <typename T>
inline void fast_decoder_base::decode_field(const T &ext_ref, group_type_tag) {
  // If a group field is optional, it will occupy a single bit in the presence
  // map.
  // The contents of the group may appear in the stream iff the bit is set.
  if (ext_ref.optional()) {
    if (!this->current_->is_next_bit_set()) {
      ext_ref.omit();
      return;
    }
  }

  decoder_pmap_saver<typename T::pmap_segment_size_type> saver(this);
  ext_ref.set().accept(*this);
}

template <typename T>
inline void fast_decoder_base::decode_field(const T &ext_ref,
                                            sequence_type_tag) {
  value_storage storage;

  auto length = ext_ref.set_length(storage);
  this->visit(length);

  if (length.present()) {
    std::size_t len = length.get().value();
    ext_ref.set().resize(len);

    for (std::size_t i = 0; i < len; ++i) {
      this->visit(ext_ref[i]);
    }
  } else {
    ext_ref.omit();
  }
}

template <typename Message>
inline void fast_decoder_base::decode_message(const message_mref &mref) {
  typename Message::mref_type ref(mref);
  ref.accept(*this);
}

template <typename T, typename TypeCategory>
void fast_decoder_base::decode_field(const T &ext_ref, none_operator_tag,
                                     TypeCategory) {
  fast_istream &stream = this->strm_;
  stream >> ext_ref;

  // Fast Specification 1.1, page 22
  //
  // If a field is mandatory and has no field operator, it will not occupy any
  // bit in the presence map and its value must always appear in the stream.
  //
  // If a field is optional and has no field operator, it is encoded with a
  // nullable representation and the NULL is used to represent absence of a
  // value. It will not occupy any bits in the presence map.
  if (ext_ref.previous_value_shared())
    save_previous_value(ext_ref.set());
}

template <typename T, typename TypeCategory>
void fast_decoder_base::decode_field(const T &ext_ref, constant_operator_tag,
                                     TypeCategory) {
  decoder_presence_map &pmap = *this->current_;
  auto mref = ext_ref.set();

  if (ext_ref.optional()) {
    // An optional field with the constant operator will occupy a single bit. If
    // the bit is set,
    // the value
    // is the initial value in the instruction context. If the bit is not set,
    // the value is
    // considered absent.

    if (pmap.is_next_bit_set()) {
      mref.to_initial_value();
    } else {
      ext_ref.omit();
    }
  } else {
    // A field will not occupy any bit in the presence map if it is mandatory
    // and has the
    // constant operator.
    // mref.to_initial_value();
  }
  if (ext_ref.previous_value_shared())
    save_previous_value(mref);
}

template <typename T, typename TypeCategory>
void fast_decoder_base::decode_field(const T &ext_ref, copy_operator_tag,
                                     TypeCategory) {
  fast_istream &stream = this->strm_;
  decoder_presence_map &pmap = *this->current_;
  auto mref = ext_ref.set();

  if (pmap.is_next_bit_set()) {
    stream >> ext_ref;
    // A NULL indicates that the value is absent and the state of the previous
    // value is set to
    // empty
    save_previous_value(mref);
  } else {
    value_storage &previous = previous_value_of(mref);

    if (!previous.is_defined()) {
      // if the previous value is undefined – the value of the field is the
      // initial value
      // that also becomes the new previous value.

      // If the field has optional presence and no initial value, the field is
      // considered
      // absent and the state of the previous value is changed to empty.
      mref.to_initial_value();
      save_previous_value(mref);

      if (ext_ref.mandatory_without_initial_value()) {
        // Unless the field has optional presence, it is a dynamic error [ERR
        // D5]
        // if the instruction context has no initial value.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D5"));
      }
    } else if (previous.is_empty()) {
      if (ext_ref.optional()) {
        // if the previous value is empty – the value of the field is empty.
        // If the field is optional the value is considered absent.
        mref.omit();
      } else {
        // It is a dynamic error [ERR D6] if the field is mandatory.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
      }

    } else {
      // if the previous value is assigned – the value of the field is the
      // previous value.
      load_previous_value(mref);
    }
  }
}

template <typename T, typename TypeCategory>
void fast_decoder_base::decode_field(const T &ext_ref, increment_operator_tag,
                                     TypeCategory) {
  fast_istream &stream = this->strm_;
  decoder_presence_map &pmap = *this->current_;
  auto mref = ext_ref.set();

  if (pmap.is_next_bit_set()) {
    stream >> ext_ref;
    // A NULL indicates that the value is absent and the state of the previous
    // value is set to
    // empty
    save_previous_value(mref);
  } else {
    value_storage &previous = previous_value_of(mref);

    if (!previous.is_defined()) {
      // if the previous value is undefined – the value of the field is the
      // initial value
      // that also becomes the new previous value.

      // If the field has optional presence and no initial value, the field is
      // considered
      // absent and the state of the previous value is changed to empty.
      mref.to_initial_value();
      save_previous_value(mref);

      if (ext_ref.mandatory_without_initial_value()) {
        // Unless the field has optional presence, it is a dynamic error [ERR
        // D5]
        // if the instruction context has no initial value.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D5"));
      }
    } else if (previous.is_empty()) {
      // if the previous value is empty – the value of the field is empty.
      if (ext_ref.optional()) {
        // If the field is optional the value is considered absent.
        mref.omit();
      } else {
        // It is a dynamic error [ERR D6] if the field is mandatory.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
      }

    } else {
      typedef typename T::mref_type::value_type value_type;
      previous.set<value_type>(previous.get<value_type>() + 1);
      // if the previous value is assigned – the value of the field is the
      // previous value.
      load_previous_value(mref);
    }
  }
}

template <typename T, typename TypeCategory>
void fast_decoder_base::decode_field(const T &ext_ref, default_operator_tag,
                                     TypeCategory) {
  fast_istream &stream = this->strm_;
  decoder_presence_map &pmap = *this->current_;
  auto mref = ext_ref.set();

  // Mandatory integer, decimal, string and byte vector fields – one bit. If
  // set, the value
  // appears in the stream.
  // Optional integer, decimal, string and byte vector fields – one bit. If set,
  // the value appears
  // in the stream in a nullable representation.

  if (pmap.is_next_bit_set()) {
    stream >> ext_ref;
    //  A NULL indicates that the value is absent and the state of the previous
    //  value is left
    //  unchanged.
    if (!ext_ref.present())
      return;
  } else {
    // If the field has optional presence and no initial value, the field is
    // considered absent
    // when there is no value in the stream.

    //  The default operator specifies that the value of a field is either
    //  present in the stream
    //  or it will be the initial value.
    mref.to_initial_value();
  }

  if (ext_ref.previous_value_shared())
    save_previous_value(mref);
}

template <typename T>
void fast_decoder_base::decode_field(const T &ext_ref, delta_operator_tag,
                                     integer_type_tag) {
  fast_istream &stream = this->strm_;
  auto mref = ext_ref.set();
  typedef typename T::mref_type::value_type int_type;

  int64_t d;
  if (stream.decode(d, ext_ref.nullable())) {
    value_storage bv = delta_base_value_of(mref);
    typename T::mref_type tmp(nullptr, &bv, nullptr);

    // check_overflow(tmp.value(), d, mref.instruction(), stream);
    mref.as(static_cast<int_type>(tmp.value() + d));

    save_previous_value(mref);
  } else {
    //  If the field has optional presence, the delta value can be NULL. In that
    //  case the value
    //  of the field is considered absent.
    mref.omit();
  }
}

template <typename T>
void fast_decoder_base::decode_field(const T &ext_ref, delta_operator_tag,
                                     string_type_tag) {
  fast_istream &stream = this->strm_;
  auto mref = ext_ref.set();
  // The delta value is represented as a Signed Integer subtraction length
  // followed by an ASCII
  // String.
  // If the delta is nullable, the subtraction length is nullable. A NULL delta
  // is represented as
  // a
  // NULL subtraction length. The string part is present in the stream iff the
  // subtraction length
  // is not NULL.
  int32_t substraction_length;
  if (stream.decode(substraction_length, ext_ref.nullable())) {
    // It is a dynamic error [ERR D7] if the subtraction length is larger than
    // the
    // number of characters in the base value, or if it does not fall in the
    // value range of an
    // int32.
    int32_t sub_len =
        substraction_length >= 0 ? substraction_length : ~substraction_length;
    const value_storage &base_value = delta_base_value_of(mref);

    if (sub_len > static_cast<int32_t>(base_value.array_length()))
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D7"));

    uint32_t delta_len;
    const typename T::mref_type::value_type *delta_str = nullptr;
    stream.decode(delta_str, delta_len, mref.instruction(), false_type());

    this->apply_string_delta(mref, base_value, substraction_length, delta_str,
                             delta_len);
    save_previous_value(mref);
  } else {
    mref.omit();
  }
}

template <typename T>
void fast_decoder_base::decode_field(const T &ext_ref, delta_operator_tag,
                                     decimal_type_tag) {
  fast_istream &stream = this->strm_;
  decimal_mref mref = ext_ref.set();
  stream >> ext_ref;
  if (!ext_ref.optional() || mref.present()) {
    value_storage bv = delta_base_value_of(mref);

    // check_overflow(bv.of_decimal.mantissa_, mref.mantissa(),
    // mref.instruction(), stream);
    // check_overflow(bv.of_decimal.exponent_, mref.exponent(),
    // mref.instruction(), stream);
    mref.set_mantissa(bv.of_decimal.mantissa_ + mref.mantissa());
    mref.set_exponent(bv.of_decimal.exponent_ + mref.exponent());
    // if (mref.exponent() > 63 || mref.exponent() < -63 )
    //   BOOST_THROW_EXCEPTION(fast_reportable_error("R1"));
    //
    save_previous_value(mref);
  }
}

template <typename T>
void fast_decoder_base::decode_field(const T &ext_ref, tail_operator_tag,
                                     string_type_tag) {
  fast_istream &stream = this->strm_;
  decoder_presence_map &pmap = *this->current_;

  auto mref = ext_ref.set();

  if (pmap.is_next_bit_set()) {
    uint32_t len;
    const typename T::mref_type::value_type *str;
    if (stream.decode(str, len, mref.instruction(), ext_ref.nullable())) {
      const value_storage &base_value(tail_base_value_of(mref));
      this->apply_string_delta(mref, base_value,
                               std::min<int>(len, base_value.array_length()),
                               str, len);
    } else {
      // If the field has optional presence, the tail value can be NULL.
      // In that case the value of the field is considered absent.
      mref.omit();
    }
  } else {
    // If the tail value is not present in the stream, the value of the field
    // depends
    // on the state of the previous value in the following way:

    auto &prev = previous_value_of(mref);

    if (!prev.is_defined()) {
      //  * undefined – the value of the field is the initial value that also
      //  becomes the new
      //  previous value.

      // If the field has optional presence and no initial value, the field is
      // considered
      // absent and the state of the previous value is changed to empty.
      mref.to_initial_value();

      if (ext_ref.mandatory_without_initial_value()) {
        // Unless the field has optional presence, it is a dynamic error [ERR
        // D6] if the
        // instruction context has no initial value.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
      }
    } else if (prev.is_empty()) {
      //  * empty – the value of the field is empty. If the field is optional
      //  the value is
      //  considered absent.
      //            It is a dynamic error [ERR D7] if the field is mandatory.
      if (ext_ref.optional())
        mref.omit();
      else
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D7"));
    } else {
      // * assigned – the value of the field is the previous value.
      load_previous_value(mref);
      return;
    }
  }
  save_previous_value(mref);
}

/////////////////////////////////////////////////////////
template <unsigned NumTokens>
inline fast_decoder_core<NumTokens>::fast_decoder_core(allocator *alloc)
    : fast_decoder_base(alloc),
      repo_(info_entry_converter(alloc), NumTokens == 0 ? nullptr : alloc),
      active_message_info_(nullptr) {}

template <unsigned NumTokens>
void fast_decoder_core<NumTokens>::visit(const nested_message_mref &mref) {
  decoder_pmap_saver<true_type> saver(this);
  info_entry *saved_active_info = this->active_message_info_;

  if (this->current_->is_next_bit_set()) {
    uint32_t template_id;

    strm_.decode(template_id, false_type());
    // find the message with corresponding template id
    active_message_info_ = repo_.find(template_id);

    if (active_message_info_ == nullptr) {
      BOOST_THROW_EXCEPTION(
          fast_dynamic_error("D9")
          << template_id_info(template_id)
          << referenced_by_info(this->active_message().name()));
    }
  }

  mref.set_target_instruction(this->active_message().instruction(),
                              false_type());
  message_decode_function_t decode = active_message_info_->decode_fun_;
  (this->*decode)(mref.target());

  this->active_message_info_ = saved_active_info;
}

template <unsigned NumTokens>
const message_mref &
fast_decoder_core<NumTokens>::decode_segment(fast_istreambuf &sb) {
  strm_.reset(&sb);

  decoder_presence_map pmap;
  this->current_ = &pmap;
  strm_.decode(pmap);

  uint32_t template_id = 0;

  if (pmap.is_next_bit_set()) {
    strm_.decode(template_id, false_type());

    // find the message with corresponding template id
    active_message_info_ = repo_.find(template_id);
  }

  if (active_message_info_ == nullptr) {
    BOOST_THROW_EXCEPTION(fast_dynamic_error("D9")
                          << coder::template_id_info(template_id));
  }

  // we have to keep the active_message_ in a new variable
  // because after the accept_mutator(), the active_message_
  // may change because of the decoding of dynamic template reference
  const message_mref &message = this->active_message();

  if (force_reset_ || message.instruction()->has_reset_attribute()) {
    repo_.reset_dictionary();
  }

  message_decode_function_t decode = active_message_info_->decode_fun_;
  (this->*decode)(message);

  return message;
}

template <unsigned NumTokens>
const message_mref &fast_decoder_core<NumTokens>::decode_stream(
    unsigned token, const char *&first, const char *last, bool force_reset) {
  assert(first < last);
  fast_istreambuf sb(first, last - first);
  this->set_token(token);
  this->force_reset_ = force_reset;
  const auto &result = this->decode_segment(sb);
  first = sb.gptr();
  return result;
}

} /* coder */

} /* mfast */
