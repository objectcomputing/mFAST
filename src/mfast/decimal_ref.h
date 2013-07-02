// Copyright (c) 2013, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
//
//     mFAST is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     mFAST is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public License
//     along with mFast.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef DECIMAL_REF_H_V02M5O0J
#define DECIMAL_REF_H_V02M5O0J

#include <cmath>
#include <cfloat>
#include "mfast/field_ref.h"
#include "mfast/int_ref.h"

namespace mfast {

class allocator;

namespace detail {
class codec_helper;
}


class decimal_cref
  : public field_cref
{
  public:
    typedef decimal_field_instruction instruction_type;
    typedef const instruction_type* instruction_cptr;

    decimal_cref()
    {
    }

    decimal_cref(const value_storage_t*   storage,
                 const field_instruction* instruction)
      : field_cref(storage, instruction)
    {
    }

    decimal_cref(const decimal_cref& other)
      : field_cref(other)
    {
    }

    uint32_t id() const
    {
      return instruction_->id();
    }

    int64_t mantissa() const
    {
      return this->storage()->decimal_storage.mantissa_;
    }

    int16_t exponent() const
    {
      return static_cast<int8_t>(this->storage()->decimal_storage.exponent_);
    }

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(instruction_);
    }

    value_storage_t default_base_value() const
    {
      value_storage_t v;
      v.defined(true);
      v.present(true);
      return v;
    }

};



class exponent_mref
  : public field_cref
{
  public:
    typedef decimal_field_instruction instruction_type;
    typedef const decimal_field_instruction* instruction_cptr;
    typedef int8_t value_type;

    exponent_mref()
    {
    }

    exponent_mref(allocator                *               /* alloc */,
                  value_storage_t*         storage,
                  const field_instruction* instruction)
      : field_cref(storage, instruction)
    {
    }

    exponent_mref(const exponent_mref& other)
      : field_cref(other)
    {
    }

    instruction_cptr instruction() const
    {
      return static_cast<instruction_cptr>(instruction_);
    }

    void as_absent() const
    {
      this->storage()->present(false);
    }

    void as_initial_value() const
    {
      as(this->instruction()->exponent_initial_value());
    }

    void as(int16_t v) const
    {
      this->storage()->decimal_storage.exponent_ = v;
      this->storage()->present(true);
    }

    int16_t value() const
    {
      return this->storage()->decimal_storage.exponent_;
    }

    value_storage_t default_base_value() const
    {
      value_storage_t v;
      v.defined(true);
      v.present(true);
      return v;
    }

  private:
    friend fast_istream& operator >> (fast_istream& strm, const exponent_mref& mref);
    int16_t& value_ref() const
    {
      this->storage()->present(true);
      return this->storage()->decimal_storage.exponent_;
    }

    value_storage_t* storage() const
    {
      return const_cast<value_storage_t*>(field_cref::storage());
    }

    friend class mfast::detail::codec_helper;

    void copy_from(value_storage_t v) const
    {
      as(v.decimal_storage.exponent_);
    }

    void save_to(value_storage_t& v) const
    {
      v.decimal_storage.exponent_ = value();
      v.defined(true);
      v.present(this->present());
    }

};

class fast_istream;

class decimal_mref
  : public make_field_mref<decimal_cref>
{
  typedef make_field_mref<decimal_cref> base_type;

  public:
    typedef const decimal_field_instruction* instruction_cptr;

    decimal_mref()
    {
    }

    decimal_mref(allocator*       alloc,
                 value_storage_t* storage,
                 instruction_cptr instruction)
      : base_type(alloc, storage, instruction)
    {
    }

    decimal_mref(const decimal_mref& other)
      : base_type(other)
    {
    }

    int64_t mantissa() const
    {
      return this->storage()->decimal_storage.mantissa_;
    }

    int8_t exponent() const
    {
      return static_cast<int8_t>(this->storage()->decimal_storage.exponent_);
    }

    void as(int64_t mant, int16_t exp) const
    {
      assert (exp <= 64 && exp >= -64);
      this->storage()->decimal_storage.mantissa_ = mant;
      this->storage()->decimal_storage.exponent_ = exp;
      this->storage()->present(1);
    }

    void set_mantissa(int64_t v) const
    {
      this->storage()->present(1);
      this->storage()->decimal_storage.mantissa_ = v;
    }

    void set_exponent(int32_t v) const
    {
      this->storage()->present(1);
      this->storage()->decimal_storage.exponent_ = v;
    }

    void as_initial_value() const
    {
      this->storage()->decimal_storage.mantissa_ = instruction()->mantissa_initial_value();
      this->storage()->decimal_storage.exponent_ = instruction()->exponent_initial_value();
      this->storage()->present(1);
    }

    exponent_mref for_exponent() const
    {
      return exponent_mref(0, this->storage(), this->instruction());
    }

    int64_mref for_mantissa() const
    {
      return int64_mref(0, this->storage(), this->instruction()->mantissa_instruction());
    }

    bool has_individual_operators() const
    {
      return this->instruction()->field_type() == field_type_exponent;
    }

  private:
    friend fast_istream& operator >> (fast_istream& strm, const decimal_mref& mref);
    friend class mfast::detail::codec_helper;

    void copy_from(value_storage_t v) const
    {
      *this->storage() = v;
    }

    void save_to(value_storage_t& v) const
    {
      v.decimal_storage.exponent_ = exponent();
      v.decimal_storage.mantissa_ = mantissa();
      v.defined(true);
      v.present(this->present());
    }

};

}

#endif /* end of include guard: DECIMAL_REF_H_V02M5O0J */
