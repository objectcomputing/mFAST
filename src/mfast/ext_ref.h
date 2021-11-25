// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include "aggregate_ref.h"
#include "sequence_ref.h"
#include "decimal_ref.h"
#include "nested_message_ref.h"

namespace mfast {
template <int V> struct fast_operator_tag : std::integral_constant<int, V> {};

template <int V> struct properties_type : std::integral_constant<int, V> {};

typedef fast_operator_tag<operator_none> none_operator_tag;
typedef fast_operator_tag<operator_constant> constant_operator_tag;
typedef fast_operator_tag<operator_delta> delta_operator_tag;
typedef fast_operator_tag<operator_default> default_operator_tag;
typedef fast_operator_tag<operator_copy> copy_operator_tag;
typedef fast_operator_tag<operator_increment> increment_operator_tag;
typedef fast_operator_tag<operator_tail> tail_operator_tag;
typedef fast_operator_tag<operators_count> invalid_operator_tag;

struct sequence_element_tag {};

// special tag for group to prevent
// a group_mref decay to an aggregate_mref
// which would lose the ability to reset
// the presence flag
//
// typedef std::integral_constant<bool, true>  true_type;
// typedef std::integral_constant<bool, false>  false_type;

using std::true_type;
using std::false_type;

template <typename Properties> struct presence_checker {
  typedef std::integral_constant<
      bool, ((Properties::value & presence_optional) > 0)> is_optional_type;

  is_optional_type optional() const { return is_optional_type(); }
};

template <typename OperatorType, typename Properties>
struct ext_ref_properties : presence_checker<Properties> {
  const static bool IsOptional =
      presence_checker<Properties>::is_optional_type::value;
  typedef Properties properties_type;

  typedef std::integral_constant<
      bool, ((Properties::value & presence_optional) > 0)> is_optional_type;
  typedef std::integral_constant<bool,
                                 (OperatorType::value != operator_constant &&
                                  IsOptional)> is_nullable_type;
  typedef std::integral_constant<
      bool, ((OperatorType::value & field_has_initial_value) > 0 &&
             !IsOptional)> mandatory_without_initial_value_type;
  typedef std::integral_constant<bool,
                                 (OperatorType::value > operator_delta ||
                                  ((OperatorType::value == operator_constant) &&
                                   IsOptional))> has_pmap_type;
  typedef std::integral_constant<bool, (Properties::value ==
                                        field_previous_value_shared)>
      previous_value_shared_type;

  is_nullable_type nullable() const { return is_nullable_type(); }
  mandatory_without_initial_value_type mandatory_without_initial_value() const {
    return mandatory_without_initial_value_type();
  }

  has_pmap_type has_pmap() const { return has_pmap_type(); }
  previous_value_shared_type previous_value_shared() const {
    return previous_value_shared_type();
  }
};

typedef std::integral_constant<unsigned int, 0> pmap_segment_size_zero;

template <typename Properties>
struct ext_ref_properties<group_type_tag, Properties>
    : presence_checker<Properties> {
  typedef std::integral_constant<unsigned int, (Properties::value >> 1)>
      pmap_segment_size_type;
  typedef Properties properties_type;

  pmap_segment_size_type pmap_segment_size() const {
    return pmap_segment_size_type();
  }
};

template <typename BaseCRef, typename OpType, typename Properties>
class ext_cref : public ext_ref_properties<OpType, Properties> {
public:
  typedef BaseCRef cref_type;
  typedef typename cref_type::type_category type_category;
  typedef OpType operator_category;

  explicit ext_cref(const field_cref &base) : base_(base) {}
  explicit ext_cref(const aggregate_cref &base) : base_(base) {}
  cref_type get() const { return base_; }
  bool present() const { return !this->optional() || base_.present(); }

private:
  cref_type base_;
};

template <typename ExponentOp, typename MantissaOp, typename Properties>
class ext_cref<decimal_cref, std::tuple<ExponentOp, MantissaOp>, Properties> {
public:
  typedef decimal_cref cref_type;
  typedef typename std::tuple_element<0, Properties>::type exponent_properties;
  typedef typename std::tuple_element<1, Properties>::type mantissa_properties;
  typedef split_decimal_type_tag type_category;
  typedef invalid_operator_tag operator_category;

  typedef ext_cref<exponent_cref, ExponentOp, exponent_properties>
      exponent_type;
  typedef ext_cref<int64_cref, MantissaOp, mantissa_properties> mantissa_type;

  explicit ext_cref(const field_cref &other) : base_(other) {}
  cref_type get() const { return base_; }
  mantissa_type get_mantissa() const {
    return mantissa_type(base_.get_mantissa());
  }
  exponent_type get_exponent() const {
    return exponent_type(base_.get_exponent());
  }
  bool present() const { return !this->optional() || base_.present(); }

private:
  decimal_cref base_;
};

template <typename LengthExtRef, typename ElementExtRef>
class ext_cref<sequence_cref, LengthExtRef, ElementExtRef> {
public:
  typedef sequence_cref cref_type;
  typedef sequence_type_tag type_category;
  typedef invalid_operator_tag operator_category;

  typedef LengthExtRef length_type;
  typedef ElementExtRef element_type;

  explicit ext_cref(const field_cref &other) : base_(other) {}
  cref_type get() const { return base_; }
  length_type get_length(value_storage &storage) const {
    uint32_mref length_mref(nullptr, &storage,
                            base_.instruction()->length_instruction());
    length_mref.as(base_.size());
    return length_type(length_mref);
  }

  element_type operator[](std::size_t i) const {
    return element_type(base_[i]);
  }
  std::size_t size() const { return base_.size(); }

private:
  sequence_cref base_;
};

template <typename BaseCRef, typename Properties>
class ext_cref<BaseCRef, sequence_element_tag, Properties>
    : public ext_ref_properties<group_type_tag, Properties> {
public:
  typedef BaseCRef cref_type;
  typedef group_type_tag type_category;
  typedef invalid_operator_tag operator_category;

  explicit ext_cref(const aggregate_cref &other) : base_(other) {}
  cref_type get() const {
    return cref_type(aggregate_cref_core_access::storage_of(base_),
                     static_cast<typename cref_type::instruction_cptr>(
                         this->base_.instruction()));
  }

  bool present() const { return true; }

protected:
  aggregate_cref base_;
};

template <typename Properties>
class ext_cref<nested_message_cref, group_type_tag, Properties>
    : public ext_ref_properties<group_type_tag, Properties> {
public:
  typedef group_type_tag type_category;
  typedef invalid_operator_tag operator_category;
  typedef nested_message_cref cref_type;

  explicit ext_cref(const field_cref &other) : base_(other) {}
  cref_type get() const { return cref_type(aggregate_cref(base_)[0]); }
  bool present() const { return !this->optional() || base_.present(); }

private:
  field_cref base_;
};

///////////////////////////////////////////////////////////////

template <typename BaseMRef, typename OpType, typename Properties>
class ext_mref : public ext_ref_properties<OpType, Properties> {
public:
  typedef BaseMRef mref_type;
  typedef typename mref_type::cref_type cref_type;
  typedef typename cref_type::type_category type_category;
  typedef OpType operator_category;

  ext_mref(field_mref other) : base_(std::move(other)) {}
  mref_type set() const {
    if (this->optional()) {
      value_storage *storage = field_mref_core_access::storage_of(this->base_);
      storage->present(true);
    }
    return mref_type(base_);
    ;
  }

  cref_type get() const {
    return cref_type(base_);
    ;
  }

  bool present() const { return !this->optional() || base_.present(); }
  void omit() const {
    if (this->optional())
      base_.omit();
  }

private:
  field_mref base_;
};

template <typename ExponentOp, typename MantissaOp, typename Properties>
class ext_mref<decimal_mref, std::tuple<ExponentOp, MantissaOp>, Properties> {
public:
  typedef decimal_mref mref_type;
  typedef decimal_cref cref_type;
  typedef typename std::tuple_element<0, Properties>::type exponent_properties;
  typedef typename std::tuple_element<1, Properties>::type mantissa_properties;
  typedef invalid_operator_tag operator_category;

  typedef ext_mref<exponent_mref, ExponentOp, exponent_properties>
      exponent_type;
  typedef ext_mref<int64_mref, MantissaOp, mantissa_properties> mantissa_type;
  typedef split_decimal_type_tag type_category;

  explicit ext_mref(const field_mref_base &other) : base_(other) {}
  mref_type set() const { return base_; }
  cref_type get() const { return base_; }
  mantissa_type set_mantissa() const {
    return mantissa_type(base_.set_mantissa());
  }
  exponent_type set_exponent() const {
    return exponent_type(base_.set_exponent());
  }

private:
  decimal_mref base_;
};

template <typename BaseMRef, typename Properties>
class ext_mref<BaseMRef, sequence_element_tag, Properties>
    : public ext_ref_properties<group_type_tag, Properties> {
public:
  typedef BaseMRef mref_type;
  typedef typename mref_type::cref_type cref_type;
  typedef group_type_tag type_category;
  typedef invalid_operator_tag operator_category;

  explicit ext_mref(const aggregate_mref &other) : base_(other) {}
  mref_type set() const {
    return mref_type(this->base_.allocator(),
                     aggregate_mref_core_access::storage_of(base_),
                     static_cast<typename mref_type::instruction_cptr>(
                         this->base_.instruction()));
  }

  cref_type get() const {
    return cref_type(aggregate_mref_core_access::storage_of(base_),
                     static_cast<typename mref_type::instruction_cptr>(
                         this->base_.instruction()));
  }

  bool present() const { return true; }
  void omit() const {}

protected:
  aggregate_mref base_;
};

template <typename LengthExtRef, typename ElementExtRef>
class ext_mref<sequence_mref, LengthExtRef, ElementExtRef> {
public:
  typedef sequence_mref mref_type;
  typedef sequence_cref cref_type;
  typedef sequence_type_tag type_category;
  typedef invalid_operator_tag operator_category;

  typedef LengthExtRef length_type;
  typedef ElementExtRef element_type;

  typedef typename length_type::is_optional_type is_optional_type;

  explicit ext_mref(const field_mref_base &other) : base_(other) {}
  mref_type set() const { return base_; }
  cref_type get() const { return base_; }
  is_optional_type optional() const { return is_optional_type(); }
  length_type set_length(value_storage &storage) const {
    field_mref_base length_mref(nullptr, &storage,
                                base_.instruction()->length_instruction());
    return length_type(length_mref);
  }

  element_type operator[](std::size_t i) const {
    return element_type(base_[i]);
  }
  std::size_t size() const { return base_.size(); }
  bool present() const { return !optional() || base_.present(); }
  void omit() const {
    if (optional())
      base_.omit();
  }

private:
  sequence_mref base_;
};

template <typename Properties>
class ext_mref<nested_message_mref, group_type_tag, Properties>
    : public ext_ref_properties<group_type_tag, Properties> {
public:
  typedef group_type_tag type_category;
  typedef invalid_operator_tag operator_category;
  typedef nested_message_cref cref_type;
  typedef nested_message_mref mref_type;

  explicit ext_mref(field_mref other) : base_(std::move(other)) {}
  cref_type get() const { return cref_type(aggregate_cref(base_)[0]); }
  mref_type set() const {
    if (this->optional()) {
      value_storage *storage = field_mref_core_access::storage_of(this->base_);
      storage->present(true);
    }
    return mref_type(aggregate_mref(base_)[0]);
  }

  bool present() const { return !this->optional() || base_.present(); }
  void omit() const {
    if (this->optional())
      base_.omit();
  }

private:
  field_mref base_;
};

} /* mfast */
