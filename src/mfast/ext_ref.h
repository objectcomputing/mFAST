#ifndef FIELD_OPERATOR_TYPE_H_2FD38AE6
#define FIELD_OPERATOR_TYPE_H_2FD38AE6

#include "aggregate_ref.h"
#include "sequence_ref.h"
#include "decimal_ref.h"
#include "nested_message_ref.h"

namespace mfast
{

  template <int V>
  struct fast_operator_type
    : boost::integral_constant<int, V>
  {
  };

  template <int V>
  struct properties_type
    : boost::integral_constant<int, V>
  {
  };


  typedef fast_operator_type<operator_none> none_operator_type;
  typedef fast_operator_type<operator_constant> constant_operator_type;
  typedef fast_operator_type<operator_delta> delta_operator_type;
  typedef fast_operator_type<operator_default> default_operator_type;
  typedef fast_operator_type<operator_copy> copy_operator_type;
  typedef fast_operator_type<operator_increment> increment_operator_type;
  typedef fast_operator_type<operator_tail> tail_operator_type;

  // special tag for group to prevent
  // a group_mref decay to an aggregate_mref
  // which would lose the ability to reset
  // the presence flag
  struct group_type_tag {};

  typedef boost::integral_constant<bool, true>  true_type;
  typedef boost::integral_constant<bool, false>  false_type;


  template <typename Properties>
  struct presence_checker
  {
    typedef boost::integral_constant<bool, ((Properties::value & presence_optional) > 0)> is_optional_type;

    is_optional_type optional() const
    {
      return is_optional_type();
    }

  };

  template <typename OperatorType, typename Properties>
  struct ext_ref_properties
    : presence_checker<Properties>
  {
    const static bool IsOptional = presence_checker<Properties>::is_optional_type::value;
    typedef Properties properties_type;

    typedef boost::integral_constant<bool, ((Properties::value & presence_optional) > 0)> is_optional_type;
    typedef boost::integral_constant<bool, (OperatorType::value != operator_constant && IsOptional )> is_nullable_type;
    typedef boost::integral_constant<bool, ((OperatorType::value & field_has_initial_value) > 0 && !IsOptional )> mandatory_without_initial_value_type;
    typedef boost::integral_constant<bool, (OperatorType::value > operator_delta || ((OperatorType::value == operator_constant) && IsOptional )) > has_pmap_type;

    is_nullable_type nullable() const
    {
      return is_nullable_type();
    }

    mandatory_without_initial_value_type mandatory_without_initial_value() const
    {
      return mandatory_without_initial_value_type();
    }

    has_pmap_type has_pmap() const
    {
      return has_pmap_type();
    }

  };

  template <typename Properties>
  struct ext_ref_properties<group_type_tag, Properties>
    : presence_checker<Properties>
  {
    typedef boost::integral_constant<bool, ((Properties::value & field_has_initial_value) > 0) > has_pmap_type;
    typedef Properties properties_type;

    has_pmap_type has_pmap() const
    {
      return has_pmap_type();
    }
  };


  template <typename BaseCRef, typename OpType, typename Properties>
  class ext_cref
    : public ext_ref_properties<OpType, Properties>
  {
  public:
    typedef BaseCRef base_type;

    explicit ext_cref(const field_cref& base)
      : base_(base)
    {
    }

    explicit ext_cref(const aggregate_cref& base)
      : base_(base)
    {
    }

    base_type base() const
    {
      return base_;
    }

    bool present() const
    {
      return !this->optional() || base_.present();
    }

  private:
    base_type base_;
  };

  template <typename Properties>
  class ext_cref<nested_message_cref, group_type_tag, Properties>
    : public ext_ref_properties<group_type_tag, Properties>
  {
  public:

    typedef nested_message_cref base_type;

    explicit ext_cref(const field_cref& other)
      :  base_(other)
    {
    }

    base_type base() const
    {
      return nested_message_cref(aggregate_cref(base_)[0]);
    }

    bool present() const
    {
      return !this->optional() || base_.present();
    }

  private:
    field_cref base_;
  };


  template <typename ExponentOp, typename MantissaOp, typename Properties>
  class ext_cref<decimal_cref, boost::mpl::pair<ExponentOp,MantissaOp>,  Properties>
  {
  public:

    typedef decimal_cref base_type;
    typedef typename Properties::first exponent_properties;
    typedef typename Properties::second mantissa_properties;


    typedef ext_cref<exponent_cref, ExponentOp, exponent_properties > exponent_type;
    typedef ext_cref<int64_cref, MantissaOp, mantissa_properties > mantissa_type;

    explicit ext_cref(const field_cref& other)
      : base_(other)
    {
    }

    base_type base() const
    {
      return base_;
    }

    mantissa_type
    get_mantissa() const
    {
      return mantissa_type(base_.get_mantissa());
    }

    exponent_type
    get_exponent() const
    {
      return exponent_type(base_.get_exponent());
    }

    bool present() const
    {
      return !this->optional() || base_.present();
    }

  private:
    decimal_cref base_;
  };


  template <typename LengthExtRef, typename ElementExtRef>
  class ext_cref<sequence_cref, LengthExtRef, ElementExtRef>
  {
  public:

    typedef sequence_cref base_type;


    typedef LengthExtRef length_type;
    typedef ElementExtRef element_type;

    explicit ext_cref(const field_cref& other)
      : base_(other)
    {
    }

    base_type base() const
    {
      return base_;
    }

    length_type
    get_length(value_storage& storage) const
    {

      uint32_cref length_cref(&storage, base_.instruction()->length_instruction());
      return length_type(length_cref);
    }

    element_type
    operator[](std::size_t i) const
    {
      return element_type(base_[i]);
    }

    std::size_t
    size() const
    {
      return base_.size();
    }

  private:
    sequence_cref base_;
  };

///////////////////////////////////////////////////////////////

  template <typename BaseMRef, typename OpType, typename Properties>
  class ext_mref
    : public ext_ref_properties<OpType, Properties>
  {
  public:

    typedef BaseMRef base_type;

    explicit ext_mref(const field_mref_base& other)
      : base_(other)
    {
    }

    explicit ext_mref(const BaseMRef& other)
      : base_(other)
    {
    }

    base_type base() const
    {
      return base_;
    }

    bool present() const
    {
      return !this->optional() || base_.present();
    }

    void omit() const
    {
      if (this->optional())
        base_.omit();
    }

  private:
    BaseMRef base_;
  };


  template <typename ExponentOp, typename MantissaOp, typename Properties>
  class ext_mref<decimal_mref, boost::mpl::pair<ExponentOp,MantissaOp>, Properties >
  {
  public:

    typedef decimal_mref base_type;

    typedef typename Properties::first exponent_properties;
    typedef typename Properties::second mantissa_properties;

    typedef ext_mref<exponent_mref, ExponentOp, exponent_properties > exponent_type;
    typedef ext_mref<int64_mref, MantissaOp, mantissa_properties > mantissa_type;


    explicit ext_mref(const field_mref_base& other)
      : base_(other)
    {
    }

    base_type base() const
    {
      return base_;
    }

    mantissa_type
    set_mantissa() const
    {
      return mantissa_type(base_.set_mantissa());
    }

    exponent_type
    set_exponent() const
    {
      return exponent_type(base_.set_exponent());
    }

  private:
    decimal_mref base_;
  };



  template <typename IsOptional>
  class group_ext_mref_base;

  template <>
  class group_ext_mref_base<true_type>
  {
  public:
    group_ext_mref_base(const field_mref_base& other)
      : base_(other)
    {
    }

    bool present() const
    {
      return base_.present();
    }

    void omit() const
    {
      base_.omit();
    }

  protected:
    value_storage* storage() const
    {
      return field_mref_core_access::storage_of(this->base_)->of_group.content_;
    }

    void as_present() const
    {
      field_mref_core_access::storage_of(this->base_)->present(true);
    }

    field_mref_base base_;
  };

  template <>
  class group_ext_mref_base<false_type>
  {
  public:
    group_ext_mref_base(const field_mref_base& other)
      : base_(other)
    {
    }

    group_ext_mref_base(const aggregate_mref& other)
      : base_(other)
    {
    }

    bool present() const
    {
      return true;
    }

    void omit() const
    {
    }

  protected:

    value_storage* storage() const
    {
      return aggregate_mref_core_access::storage_of(this->base_);
    }

    void as_present() const
    {
    }

    aggregate_mref base_;
  };


  template <typename BaseMRef, typename Properties>
  class ext_mref<BaseMRef, group_type_tag, Properties>
    : public ext_ref_properties<group_type_tag, Properties>
    , public group_ext_mref_base<typename presence_checker<Properties>::is_optional_type>
  {
    typedef group_ext_mref_base<typename presence_checker<Properties>::is_optional_type> ext_base;
  public:

    typedef BaseMRef base_type;

    explicit ext_mref(const field_mref_base& other)
      :  ext_base(other)
    {
    }

    explicit ext_mref(const aggregate_mref& other)
      :  ext_base(other)
    {
    }

    base_type base() const
    {
      this->as_present();
      return base_type(this->base_.allocator(),
                       this->storage(),
                       static_cast<typename base_type::instruction_cptr>(this->base_.instruction()));
    }


  };

  template <typename Properties>
  class ext_mref<nested_message_mref, group_type_tag, Properties>
    : public ext_ref_properties<group_type_tag, Properties>
  {
  public:

    typedef nested_message_mref base_type;

    explicit ext_mref(const field_mref_base& other)
      :  base_(other)
    {
    }


    base_type base() const
    {
      return  nested_message_mref(aggregate_mref(base_)[0]);
    }

    bool present() const
    {
      return !this->optional() || base_.present();
    }

    void omit() const
    {
      if (this->optional())
        base_.omit();
    }

  private:
    field_mref base_;
  };


  template <typename LengthExtRef, typename ElementExtRef>
  class ext_mref<sequence_mref, LengthExtRef, ElementExtRef>
  {
  public:

    typedef sequence_mref base_type;


    typedef LengthExtRef length_type;
    typedef ElementExtRef element_type;

    typedef typename length_type::is_optional_type is_optional_type;


    explicit ext_mref(const field_mref_base& other)
      : base_(other)
    {
    }

    base_type base() const
    {
      return base_;
    }

    is_optional_type optional() const
    {
      return is_optional_type();
    }

    length_type
    set_length(value_storage& storage) const
    {

      field_mref_base length_mref(0, &storage, base_.instruction()->length_instruction());
      return length_type(length_mref);
    }

    element_type
    operator[](std::size_t i) const
    {
      return element_type(base_[i]);
    }

    std::size_t
    size() const
    {
      return base_.size();
    }

    bool present() const
    {
      return !optional() || base_.present();
    }

    void omit() const
    {
      if (optional())
        base_.omit();
    }

  private:
    sequence_mref base_;
  };


} /* mfast */



#endif /* end of include guard: FIELD_OPERATOR_TYPE_H_2FD38AE6 */
