// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include "../mfast_coder_export.h"
#include "../common/codec_helper.h"
#include "fast_istream.h"
#include "decoder_presence_map.h"
#include "decoder_field_operator.h"
#include "fast_istream_extractor.h"
#include "check_overflow.h"
namespace mfast {

void decoder_field_operator::decode(const int32_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

void decoder_field_operator::decode(const uint32_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

void decoder_field_operator::decode(const int64_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

void decoder_field_operator::decode(const uint64_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

void decoder_field_operator::decode(const ascii_string_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

void decoder_field_operator::decode(const unicode_string_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

void decoder_field_operator::decode(const byte_vector_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

void decoder_field_operator::decode(const decimal_mref & /* mref */,
                                    fast_istream & /* stream */,
                                    decoder_presence_map & /* pmap */) const {}

namespace decoder_detail {

template <typename Operator> struct decimal_decoder {
  void decode_decimal(const decimal_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const {
    const Operator *derived = static_cast<const Operator *>(this);
    if (!mref.has_individual_operators())
      derived->decode_impl(mref, stream, pmap);
    else {
      derived->decode_impl(mref.set_exponent(), stream, pmap);
      if (mref.present()) {
        int64_mref mantissa_mref = mref.set_mantissa();
        const decoder_field_operator *mantissa_operator =
            decoder_operators[mantissa_mref.instruction()->field_operator()];
        mantissa_operator->decode(mantissa_mref, stream, pmap);
      }
    }
  }
};

class no_operator : public decoder_field_operator,
                    public mfast::detail::codec_helper,
                    public decimal_decoder<no_operator> {
public:
  no_operator() {}

  template <typename T>
  void decode_impl(const T &mref, fast_istream &stream,
                   decoder_presence_map & /* pmap */) const {
    stream >> mref;

    // Fast Specification 1.1, page 22
    //
    // If a field is mandatory and has no field operator, it will not occupy any
    // bit in the presence map and its value must always appear in the stream.
    //
    // If a field is optional and has no field operator, it is encoded with a
    // nullable representation and the NULL is used to represent absence of a
    // value. It will not occupy any bits in the presence map.
    save_previous_value(mref);
  }

  virtual void decode(const int32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const int64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const ascii_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const unicode_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const byte_vector_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const decimal_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_decimal(mref, stream, pmap);
  }
};

class constant_operator : public decoder_field_operator,
                          public mfast::detail::codec_helper,
                          public decimal_decoder<constant_operator> {
public:
  constant_operator() {}

  template <typename T>
  void decode_impl(const T &mref, fast_istream & /* stream */,
                   decoder_presence_map &pmap) const {

    // A field will not occupy any bit in the presence map if it is mandatory
    // and has the constant operator.
    // An optional field with the constant operator will occupy a single bit. If
    // the bit is set, the value
    // is the initial value in the instruction context. If the bit is not set,
    // the value is considered absent.

    if (!mref.optional()) {
      mref.to_initial_value();
    } else {
      if (pmap.is_next_bit_set()) {
        mref.to_initial_value();
      } else {
        mref.omit();
      }
    }
    save_previous_value(mref);
  }

  virtual void decode(const int32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const int64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const ascii_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const unicode_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const byte_vector_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const decimal_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_decimal(mref, stream, pmap);
  }
};

template <typename Operation>
class copy_or_increment_operator_impl : public mfast::detail::codec_helper {
public:
  template <typename T>
  void decode_impl(const T &mref, fast_istream &stream,
                   decoder_presence_map &pmap) const {
    if (pmap.is_next_bit_set()) {
      stream >> mref;
      // A NULL indicates that the value is absent and the state of the previous
      // value is set to empty
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

        if (mref.instruction()->mandatory_without_initial_value()) {
          // Unless the field has optional presence, it is a dynamic error [ERR
          // D5]
          // if the instruction context has no initial value.
          BOOST_THROW_EXCEPTION(fast_dynamic_error("D5"));
        }
      } else if (previous.is_empty()) {

        // It is a dynamic error [ERR D6] if the field is mandatory.
        if (!mref.optional()) {
          BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
        }
        // if the previous value is empty – the value of the field is empty.
        // If the field is optional the value is considered absent.
        mref.omit();
      } else {
        Operation()(mref, previous);
        // if the previous value is assigned – the value of the field is the
        // previous value.
        load_previous_value(mref);
      }
    }
  }
};

struct null_operation {
  template <typename T> void operator()(const T &, value_storage &) const {}
};

class copy_operator : public decoder_field_operator,
                      public copy_or_increment_operator_impl<null_operation>,
                      public decimal_decoder<copy_operator> {

public:
  copy_operator() {}

  virtual void decode(const int32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const int64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const ascii_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const unicode_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const byte_vector_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const decimal_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_decimal(mref, stream, pmap);
  }
};

struct increment_operation {
  template <typename T> void operator()(T &, value_storage &previous) const {
    previous.set<typename T::value_type>(
        previous.get<typename T::value_type>() + 1);
  }
};

class increment_operator
    : public decoder_field_operator,
      copy_or_increment_operator_impl<increment_operation> {

public:
  increment_operator() {}

  virtual void decode(const int32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const int64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }
};

class default_operator : public decoder_field_operator,
                         public mfast::detail::codec_helper,
                         public decimal_decoder<default_operator> {
public:
  default_operator() {}

  template <typename T>
  void decode_impl(const T &mref, fast_istream &stream,
                   decoder_presence_map &pmap) const {
    // Mandatory integer, decimal, string and byte vector fields – one bit. If
    // set, the value appears in the stream.
    // Optional integer, decimal, string and byte vector fields – one bit. If
    // set, the value appears in the stream in a nullable representation.

    if (pmap.is_next_bit_set()) {
      stream >> mref;
      //  A NULL indicates that the value is absent and the state of the
      //  previous value is left unchanged.
      if (mref.absent())
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

    save_previous_value(mref);
  }

  virtual void decode(const int32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint32_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const int64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const uint64_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const ascii_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const unicode_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const byte_vector_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const decimal_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_decimal(mref, stream, pmap);
  }
};

class delta_operator : public decoder_field_operator,
                       public mfast::detail::codec_helper {
  template <typename T>
  void decode_integer(const T &mref, fast_istream &stream) const {
    int64_t d;
    if (stream.decode(d, mref.instruction()->is_nullable())) {

      value_storage bv = delta_base_value_of(mref);
      T tmp(nullptr, &bv, nullptr);

      check_overflow(tmp.value(), d, mref.instruction(), stream);
      mref.as(static_cast<typename T::value_type>(tmp.value() + d));

      save_previous_value(mref);
    } else {
      //  If the field has optional presence, the delta value can be NULL. In
      //  that case the value of the field is considered absent.
      mref.omit();
    }
  }

  template <typename T>
  void decode_string(const T &mref, fast_istream &stream,
                     decoder_presence_map & /* pmap */) const {
    // The delta value is represented as a Signed Integer subtraction length
    // followed by an ASCII String.
    // If the delta is nullable, the subtraction length is nullable. A NULL
    // delta is represented as a
    // NULL subtraction length. The string part is present in the stream iff the
    // subtraction length is not NULL.
    int32_t substraction_length;
    if (stream.decode(substraction_length, mref.instruction()->is_nullable())) {
      // It is a dynamic error [ERR D7] if the subtraction length is larger than
      // the
      // number of characters in the base value, or if it does not fall in the
      // value range of an int32.
      int32_t sub_len =
          substraction_length >= 0 ? substraction_length : ~substraction_length;
      const value_storage &base_value = delta_base_value_of(mref);

      if (sub_len > static_cast<int32_t>(base_value.array_length()))
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D7"));

      uint32_t delta_len;
      const typename T::value_type *delta_str = nullptr;
      stream.decode(delta_str, delta_len, mref.instruction(), false);

      this->apply_string_delta(mref, base_value, substraction_length, delta_str,
                               delta_len);
      save_previous_value(mref);
    } else {
      mref.omit();
    }
  }

public:
  delta_operator() {}

  virtual void decode(const int32_mref &mref, fast_istream &stream,
                      decoder_presence_map & /* pmap */) const override {
    this->decode_integer(mref, stream);
  }

  virtual void decode(const uint32_mref &mref, fast_istream &stream,
                      decoder_presence_map & /* pmap */) const override {
    decode_integer(mref, stream);
  }

  virtual void decode(const int64_mref &mref, fast_istream &stream,
                      decoder_presence_map & /* pmap */) const override {
    decode_integer(mref, stream);
  }

  virtual void decode(const uint64_mref &mref, fast_istream &stream,
                      decoder_presence_map & /* pmap */) const override {
    decode_integer(mref, stream);
  }

  virtual void decode(const decimal_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    if (!mref.has_individual_operators()) {
      stream >> mref;
      if (mref.present()) {
        value_storage bv = delta_base_value_of(mref);

        check_overflow(bv.of_decimal.mantissa_, mref.mantissa(),
                       mref.instruction(), stream);
        check_overflow(bv.of_decimal.exponent_, mref.exponent(),
                       mref.instruction(), stream);
        mref.set_mantissa(bv.of_decimal.mantissa_ + mref.mantissa());
        mref.set_exponent(bv.of_decimal.exponent_ + mref.exponent());
        // if (mref.exponent() > 63 || mref.exponent() < -63 )
        //   BOOST_THROW_EXCEPTION(fast_reportable_error("R1"));
        //
        save_previous_value(mref);
      } else {
        mref.omit();
      }
    } else {
      decode_integer(mref.set_exponent(), stream);
      if (mref.present()) {
        int64_mref mantissa_mref = mref.set_mantissa();
        const decoder_field_operator *mantissa_operator =
            decoder_operators[mantissa_mref.instruction()->field_operator()];
        mantissa_operator->decode(mantissa_mref, stream, pmap);
      }
    }
  }

  virtual void decode(const ascii_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_string(mref, stream, pmap);
  }

  virtual void decode(const unicode_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_string(mref, stream, pmap);
  }

  virtual void decode(const byte_vector_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_string(mref, stream, pmap);
  }
};

class tail_operator : public decoder_field_operator,
                      public mfast::detail::codec_helper {
private:
  template <typename T>
  void decode_impl(const T &mref, fast_istream &stream,
                   decoder_presence_map &pmap) const {
    if (pmap.is_next_bit_set()) {

      uint32_t len;
      const typename T::value_type *str;
      if (stream.decode(str, len, mref.instruction(),
                        mref.instruction()->is_nullable())) {
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

      value_storage &prev = previous_value_of(mref);

      if (!prev.is_defined()) {
        //  * undefined – the value of the field is the initial value that also
        //  becomes the new previous value.

        // If the field has optional presence and no initial value, the field is
        // considered absent and the state of the previous value is changed to
        // empty.
        mref.to_initial_value();

        if (mref.instruction()->mandatory_without_initial_value()) {
          // Unless the field has optional presence, it is a dynamic error [ERR
          // D6] if the instruction context has no initial value.
          BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
        }
      } else if (prev.is_empty()) {
        //  * empty – the value of the field is empty. If the field is optional
        //  the value is considered absent.
        //            It is a dynamic error [ERR D7] if the field is mandatory.
        if (!mref.optional())
          BOOST_THROW_EXCEPTION(fast_dynamic_error("D7"));
        mref.omit();
      } else {
        // * assigned – the value of the field is the previous value.
        load_previous_value(mref);
        return;
      }
    }
    save_previous_value(mref);
  }

public:
  tail_operator() {}

  virtual void decode(const ascii_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const unicode_string_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }

  virtual void decode(const byte_vector_mref &mref, fast_istream &stream,
                      decoder_presence_map &pmap) const override {
    decode_impl(mref, stream, pmap);
  }
};

static const no_operator no_operator_instance;
static const constant_operator constant_operator_instance;
static const copy_operator copy_operator_instance;
static const default_operator default_operator_instance;
static const delta_operator delta_operator_instance;
static const increment_operator increment_operator_instance;
static const tail_operator tail_operator_instance;
}

const decoder_field_operator *const decoder_operators[] = {
    &decoder_detail::no_operator_instance,
    &decoder_detail::constant_operator_instance,
    &decoder_detail::delta_operator_instance,
    &decoder_detail::default_operator_instance,
    &decoder_detail::copy_operator_instance,
    &decoder_detail::increment_operator_instance,
    &decoder_detail::tail_operator_instance};
}
