// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <cmath>
#include <cfloat>
#include <boost/utility/string_ref.hpp>
#include "mfast_export.h"
#include "field_ref.h"
#include "int_ref.h"
#include "type_category.h"
#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/spirit/home/x3/support/numeric_utils/extract_real.hpp>
#if defined __GNUC__
#pragma GCC diagnostic pop
#endif
#if defined(MAST_NO_SHARED_LIBS) && !defined(BOOST_NO_CXX11_EXTERN_TEMPLATE)
namespace boost {
namespace multiprecision {
namespace backends {
MFAST_EXTERN_TEMPLATE template class cpp_dec_float<18>;
}
}
}
#endif

namespace mfast {
typedef boost::multiprecision::backends::cpp_dec_float<18> decimal_backend;
typedef boost::multiprecision::number<decimal_backend> decimal;

inline decimal make_decimal(int64_t mantissa, int16_t exponent) {
  decimal r(mantissa);
  r *= decimal_backend(1.0, exponent);
  return r;
}

class allocator;

namespace detail {
class codec_helper;
}

class exponent_cref : public field_cref {
public:
  typedef decimal_field_instruction instruction_type;
  typedef const decimal_field_instruction *instruction_cptr;
  typedef int16_t value_type;
  typedef integer_type_tag type_category;

  exponent_cref() {}
  exponent_cref(const value_storage *storage,
                const field_instruction *instruction)
      : field_cref(storage, instruction) {}

  exponent_cref(const exponent_cref &other) : field_cref(other) {}
  explicit exponent_cref(const field_cref &other)
      : field_cref(field_cref_core_access::storage_of(other),
                   other.instruction()) {}

  instruction_cptr instruction() const {
    return static_cast<instruction_cptr>(instruction_);
  }
  bool is_initial_value() const {
    return (this->absent() == instruction()->initial_value().is_empty()) &&
           (this->absent() ||
            this->value() ==
                instruction()->initial_value().of_decimal.exponent_);
  }

  int16_t value() const { return this->storage()->of_decimal.exponent_; }
  value_storage default_base_value() const {
    value_storage v;
    v.defined(true);
    v.present(true);
    return v;
  }

private:
  friend class mfast::detail::codec_helper;

  void save_to(value_storage &v) const {
    v.of_decimal.exponent_ = value();
    v.defined(true);
    v.present(this->present());
  }
};

class decimal_cref : public field_cref {
public:
  typedef decimal_field_instruction instruction_type;
  typedef const instruction_type *instruction_cptr;
  typedef decimal value_type;
  typedef decimal_type_tag type_category;

  decimal_cref() {}
  decimal_cref(const value_storage *storage,
               const field_instruction *instruction)
      : field_cref(storage, instruction) {}

  decimal_cref(const decimal_cref &other) : field_cref(other) {}
  explicit decimal_cref(const field_cref &other) : field_cref(other) {}
  uint32_t id() const { return instruction_->id(); }
  int64_t mantissa() const { return this->storage()->of_decimal.mantissa_; }
  int16_t exponent() const {
    return static_cast<int16_t>(this->storage()->of_decimal.exponent_);
  }
  bool is_initial_value() const;

  decimal value() const {
    decimal r(mantissa());
    r *= decimal_backend(1.0, exponent());
    return r;
  }

  instruction_cptr instruction() const {
    return static_cast<instruction_cptr>(instruction_);
  }
  operator double() const {
    double x = static_cast<double>(mantissa());
    // return x * std::pow(10, exponent());
    boost::spirit::x3::extension::scale(exponent(),x);
    return x;
  }

  std::string to_string() const {
    std::stringstream strm;
    strm << *reinterpret_cast<const decimal_value_storage *>(this->storage());
    return strm.str();
  }

  value_storage default_base_value() const {
    value_storage v;
    v.defined(true);
    v.present(true);
    return v;
  }

  bool has_individual_operators() const {
    return this->instruction()->field_type() == field_type_exponent;
  }

  exponent_cref get_exponent() const {
    return exponent_cref(this->storage(), this->instruction());
  }

  int64_cref get_mantissa() const {
    return int64_cref(this->storage(),
                      this->instruction()->mantissa_instruction());
  }

private:
  friend class mfast::detail::codec_helper;

  void save_to(value_storage &v) const {
    v.of_decimal.exponent_ = exponent();
    v.of_decimal.mantissa_ = mantissa();
    v.defined(true);
    v.present(this->present());
  }
};

inline bool operator==(const decimal_cref &lhs, const decimal_cref &rhs) {
  return (lhs.absent() == rhs.absent()) &&
         (lhs.absent() || (lhs.mantissa() == rhs.mantissa() &&
                           lhs.exponent() == rhs.exponent()));
}

inline bool operator!=(const decimal_cref &lhs, const decimal_cref &rhs) {
  return !(lhs == rhs);
}
inline bool decimal_cref::is_initial_value() const {
  return *this == decimal_cref(&this->instruction()->initial_value(),
                               this->instruction());
}

class exponent_mref : public exponent_cref {
public:
  typedef exponent_cref cref_type;

  exponent_mref() {}
  exponent_mref(allocator * /* alloc */, value_storage *storage,
                const field_instruction *instruction)
      : exponent_cref(storage, instruction) {}

  exponent_mref(const exponent_mref &other) : exponent_cref(other) {}
  explicit exponent_mref(const field_mref_base &other)
      : exponent_cref(field_mref_core_access::storage_of(other),
                      other.instruction()) {}

  void omit() const { this->storage()->present(false); }
  void to_initial_value() const {
    this->storage()->of_decimal.exponent_ =
        this->instruction()->initial_value().of_decimal.exponent_;
    this->storage()->of_decimal.present_ =
        this->instruction()->initial_value().of_decimal.present_;
  }

  void as(int16_t v) const {
    assert(!has_const_exponent() ||
           v == instruction()->initial_value().of_decimal.exponent_);

    this->storage()->of_decimal.exponent_ = v;
    this->storage()->present(true);
  }

  void as(const exponent_cref &cref) const {
    if (cref.absent()) {
      omit();
    } else {
      as(cref.value());
    }
  }

private:
  friend fast_istream &operator>>(fast_istream &strm,
                                  const exponent_mref &mref);
  int16_t &value_ref() const {
    this->storage()->present(true);
    return this->storage()->of_decimal.exponent_;
  }

  value_storage *storage() const {
    return const_cast<value_storage *>(field_cref::storage());
  }
  friend class mfast::detail::codec_helper;
  friend class field_mref;

  void copy_from(value_storage v) const {
    this->storage()->of_decimal.exponent_ = v.of_decimal.exponent_;
    this->storage()->present(!v.is_empty());
  }

  bool has_const_exponent() const {
    return this->instruction()->field_type() == field_type_exponent &&
           this->instruction()->field_operator() == operator_constant;
  }
};

template <> struct mref_of<exponent_cref> { typedef exponent_mref type; };

class fast_istream;

class decimal_mref : public make_field_mref<decimal_cref> {
  typedef make_field_mref<decimal_cref> base_type;

public:
  typedef const decimal_field_instruction *instruction_cptr;

  decimal_mref() {}
  decimal_mref(allocator_type *alloc, value_storage *storage,
               instruction_cptr instruction)
      : base_type(alloc, storage, instruction) {}

  decimal_mref(const decimal_mref &) = default;

  explicit decimal_mref(const field_mref_base &other) : base_type(other) {}
  int64_t mantissa() const { return this->storage()->of_decimal.mantissa_; }
  int16_t exponent() const {
    return static_cast<int16_t>(this->storage()->of_decimal.exponent_);
  }
  template <typename T> enable_if_t<std::is_integral<T>::value> as(T v) {
    this->storage()->of_decimal.mantissa_ = v;
    this->storage()->of_decimal.exponent_ = 0;

    this->storage()->present(1);
    normalize();
  }

  template <typename T> enable_if_t<!std::is_integral<T>::value> as(T v) {
    this->as_i(v);
  }

  void as(int64_t mant, int16_t exp) const {
    assert(exp <= 64 && exp >= -64);

    this->storage()->of_decimal.mantissa_ = mant;
    this->storage()->of_decimal.exponent_ = exp;

    this->storage()->present(1);
  }

  void set_mantissa(int64_t v) const {
    this->storage()->present(1);
    this->storage()->of_decimal.mantissa_ = v;
  }

  void set_exponent(int32_t v) const {
    this->storage()->present(1);
    this->storage()->of_decimal.exponent_ = v;
  }

  void to_initial_value() const {
    *this->storage() = instruction()->initial_value();
  }
  exponent_mref set_exponent() const {
    return exponent_mref(nullptr, this->storage(), this->instruction());
  }

  int64_mref set_mantissa() const {
    return int64_mref(nullptr, this->storage(),
                      this->instruction()->mantissa_instruction());
  }

  void normalize() const {
    if (!has_const_exponent()) {
      while (mantissa() != 0 && mantissa() % 10 == 0) {
        this->storage()->of_decimal.mantissa_ /= 10;
        this->storage()->of_decimal.exponent_ += 1;
      }

      if (mantissa() == 0)
        this->set_exponent(0);
    } else {
      // we need to adjust the mantissa and exponent if the exponent is required
      // to be const

      int16_t fixed_exponent =
          instruction()->initial_value().of_decimal.exponent_;
      int16_t exponent_diff = exponent() - fixed_exponent;
      for (; exponent_diff > 0; --exponent_diff) {
        this->storage()->of_decimal.mantissa_ *= 10;
      }
      for (; exponent_diff < 0; ++exponent_diff) {
        this->storage()->of_decimal.mantissa_ /= 10;
      }
      this->storage()->of_decimal.exponent_ = fixed_exponent;
    }
  }

private:
  void as_i(decimal d) const
#ifndef _MSC_VER
  {
    this->as_i<18, boost::int32_t, void>(d);
  }

  template <unsigned Digits10, class ExponentType, class Allocator>
  void
  as_i(boost::multiprecision::number<
       boost::multiprecision::backends::cpp_dec_float<Digits10, ExponentType,
                                                      Allocator>> d) const
#endif
  {
    double m;
    int32_t exp;
    if (!has_const_exponent()) {
      d.backend().extract_parts(m, exp);
      d *= decimal(decimal_backend(1.0, 17 - exp));
      // Don't be distract by the method name -- extract_unsigned_long_long()
      // the returned value is signed
      this->storage()->of_decimal.mantissa_ =
          d.backend().extract_unsigned_long_long();
      this->storage()->of_decimal.exponent_ = exp - 17;
      normalize();
    } else {
      const int16_t const_exp =
          instruction()->initial_value().of_decimal.exponent_;
      d *= decimal(decimal_backend(1.0, -1 * const_exp));
      this->storage()->of_decimal.mantissa_ =
          d.backend().extract_unsigned_long_long();
      this->storage()->of_decimal.exponent_ = const_exp;
    }
    this->storage()->present(1);
  }

  void as_i(boost::string_ref decimal_str) const {
    typedef boost::iostreams::stream<boost::iostreams::array_source>
        array_stream;
    array_stream is(decimal_str.data(), decimal_str.size());
    is >> *reinterpret_cast<decimal_value_storage *>(this->storage());

    if (!is) {
      std::string msg(decimal_str.data(), decimal_str.size());
      msg += " is not a decimal value";
      BOOST_THROW_EXCEPTION(std::runtime_error(msg));
    }

    normalize();
  }

  void as_i(const decimal_cref &cref) const {
    if (cref.absent()) {
      omit();
    } else {
      as(cref.mantissa(), cref.exponent());
    }
  }

  bool has_const_exponent() const {
    return this->instruction()->field_type() == field_type_exponent &&
           this->instruction()->field_operator() == operator_constant;
  }

  friend fast_istream &operator>>(fast_istream &strm, const decimal_mref &mref);
  friend class mfast::detail::codec_helper;

  void copy_from(value_storage v) const { *this->storage() = v; }
};

class decimal_type {
public:
  typedef const decimal_field_instruction *instruction_cptr;
  typedef decimal_cref cref_type;
  typedef decimal_mref mref_type;

  decimal_type(mfast::allocator * = nullptr,
               instruction_cptr instruction =
                   decimal_field_instruction::default_instruction())
      : instruction_(instruction) {
    instruction_->construct_value(my_storage_, nullptr);
  }

  mref_type ref() { return mref_type(nullptr, &my_storage_, instruction_); }
  mref_type mref() { return mref_type(nullptr, &my_storage_, instruction_); }
  cref_type ref() const { return cref_type(&my_storage_, instruction_); }
  cref_type cref() const { return cref_type(&my_storage_, instruction_); }

private:
  instruction_cptr instruction_;
  value_storage my_storage_;
};

template <> struct mref_of<decimal_cref> { typedef decimal_mref type; };
}
