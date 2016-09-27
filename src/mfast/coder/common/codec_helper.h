// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "../../string_ref.h"
#include "../../exceptions.h"
#include <stdexcept>

namespace mfast {
namespace detail {
class codec_helper {
public:
  template <typename T> value_storage &previous_value_of(const T &mref) const {
    return const_cast<typename T::instruction_type *>(mref.instruction())
        ->prev_value();
  }

  template <typename T> void save_previous_value(const T &mref) const {
    mref.save_to(previous_value_of(mref));
  }

  template <typename T> void load_previous_value(const T &mref) const {
    mref.copy_from(previous_value_of(mref));
  }

  template <typename T>
  const value_storage &delta_base_value_of(const T &mref) const {
    // The base value depends on the state of the previous value in the
    // following way:
    value_storage &previous = previous_value_of(mref);

    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the
    // instruction context. Otherwise a type dependant default base value is
    // used.
    // * empty – it is a dynamic error [ERR D6] if the previous value is empty.

    if (!previous.is_defined()) {
      return mref.instruction()->initial_or_default_value();
    }

    if (previous.is_empty())
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));

    return previous;
  }

  template <typename T>
  const value_storage &tail_base_value_of(const T &mref) const {
    // The base value depends on the state of the previous value in the
    // following way:
    value_storage &previous = previous_value_of(mref);

    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the
    // instruction context. Otherwise a type dependant default base value is
    // used.
    // * empty – the base value is the initial value if present in the
    // instruction context. Otherwise a type dependant default base value is
    // used.
    if (!previous.is_defined() || previous.is_empty()) {
      return mref.instruction()->initial_or_default_value();
    }

    return previous;
  }

  void copy_string_raw(const ascii_string_mref &mref, std::size_t pos,
                       const char *str, std::size_t len) const {
    std::memcpy(&mref[pos], str, len);
    mref[pos + len - 1] &= 0x7F;
  }

  void copy_string_raw(const unicode_string_mref &mref, std::size_t pos,
                       const char *str, std::size_t len) const {
    std::memcpy(&mref[pos], str, len);
  }

  void copy_string_raw(const byte_vector_mref &mref, std::size_t pos,
                       const unsigned char *str, std::size_t len) const {
    std::memcpy(&mref[pos], str, len);
  }

  template <typename STRING_MREF>
  void apply_string_delta(const STRING_MREF &mref,
                          const value_storage &base_value,
                          int32_t substraction_length,
                          const typename STRING_MREF::value_type *delta_str,
                          uint32_t delta_len) const {
    std::size_t base_len = base_value.array_length();
    const typename STRING_MREF::value_type *base_str =
        static_cast<const typename STRING_MREF::value_type *>(
            base_value.of_array.content_);
    std::size_t delta_start_index;
    std::size_t base_start_index;

    if (substraction_length >= 0) {
      // The subtraction length of the delta specifies the number of characters
      // to remove from the front or back of the base value.
      if (base_len < static_cast<std::size_t>(substraction_length)) {
        // It is a dynamic error [ERR D7] if the subtraction length is larger
        // than the number of characters in the base
        // value, or if it does not fall in the value range of an int32.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D7"));
      }

      base_len -= substraction_length;
      delta_start_index = base_len;
      base_start_index = 0;
    } else {
      // Characters are removed from the front when the subtraction length is
      // negative.
      // The subtraction length uses an excess-1 encoding: if the value is
      // negative when decoding,
      // it is incremented by one to get the number of characters to subtract.
      // This makes it possible
      // to encode negative zero as -1,
      base_start_index = ~substraction_length;
      base_len -= base_start_index;
      base_str += base_start_index;
      delta_start_index = 0;
      base_start_index = delta_len;
    }

    mref.resize(base_len + delta_len);
    if (base_str != mref.data() && (base_len > 0)) {
      std::memmove(mref.data() + base_start_index, base_str, base_len);
    }
    if (delta_len > 0) {
      assert(delta_str != nullptr);
      copy_string_raw(mref, delta_start_index, delta_str, delta_len);
    }
  }
};
}
}
