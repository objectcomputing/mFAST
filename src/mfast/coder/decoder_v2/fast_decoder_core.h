#ifndef FAST_DECODER_CORE_H_51998563
#define FAST_DECODER_CORE_H_51998563

#include "../mfast_coder_export.h"

#include <boost/container/map.hpp>
#include "mfast/sequence_ref.h"
#include "mfast/nested_message_ref.h"
#include "mfast/malloc_allocator.h"
#include "mfast/output.h"
#include "mfast/composite_type.h"
#include "../common/exceptions.h"
#include "../common/debug_stream.h"
#include "../common/template_repo.h"
#include "../common/codec_helper.h"
#include "../decoder/decoder_presence_map.h"
#include "../common/codec_helper.h"
#include "../decoder/fast_istream.h"
#include "fast_istream_extractor.h"
#include <boost/tuple/tuple.hpp>
namespace mfast
{
namespace coder
{
struct fast_decoder_core;
typedef void (fast_decoder_core::* message_decode_function_t) (const message_mref&);


struct decoder_info_entry
{
  decoder_info_entry(const boost::tuple<allocator*,
                                        const template_instruction*,
                                        coder::message_decode_function_t>& inputs)
    : message_(inputs.get<0>(), inputs.get<1>())
    , decode_fun_(inputs.get<2>())
  {
  }

  message_type message_;
  coder::message_decode_function_t decode_fun_;
};


struct MFAST_CODER_EXPORT fast_decoder_core
  : detail::codec_helper
  , template_repo< template_repo_entry_converter<fast_decoder_core,
                                                 decoder_info_entry,
                                                 boost::tuple<mfast::allocator*,
                                                              template_instruction*,
                                                              message_decode_function_t> > >
{
  typedef template_repo< template_repo_entry_converter<fast_decoder_core,
                                                       decoder_info_entry,
                                                       boost::tuple<mfast::allocator*,
                                                                    template_instruction*,
                                                                    message_decode_function_t> > > template_repo_base;
  fast_decoder_core(allocator* alloc);

  message_type* decode_segment(fast_istreambuf& sb);
  message_cref decode_stream(const char*& first, const char* last, bool force_reset);


  template <typename Message>
  void decode_message(const message_mref& mref);

  template <typename DescriptionsTuple>
  void init(const DescriptionsTuple& tp);

  template <typename T>
  void visit(const T& ext_ref);

  void visit(const nested_message_mref& mref);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref, TypeCategory);

  template <typename T>
  void decode_field(const T &ext_ref, split_decimal_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref, int_vector_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref, group_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref, sequence_type_tag);


  template <typename T, typename TypeCategory>
  void decode_field (const T &ext_ref,
                     none_operator_tag,
                     TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref,
                    constant_operator_tag,
                    TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref,
                    copy_operator_tag,
                    TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref,
                    increment_operator_tag,
                    TypeCategory);

  template <typename T, typename TypeCategory>
  void decode_field(const T &ext_ref,
                    default_operator_tag,
                    TypeCategory);

  template <typename T>
  void decode_field(const T &ext_ref,
                    delta_operator_tag,
                    integer_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref,
                    delta_operator_tag,
                    string_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref,
                    delta_operator_tag,
                    decimal_type_tag);

  template <typename T>
  void decode_field(const T &ext_ref,
                    tail_operator_tag,
                    string_type_tag);


  typedef decoder_info_entry repo_mapped_type;



  template <typename Message>
  boost::tuple<mfast::allocator*, template_instruction*, message_decode_function_t>
  to_repo_entry(template_instruction* inst, Message*)
  {
    return boost::make_tuple(this->message_alloc_, inst, &fast_decoder_core::decode_message<Message>);
  }

  template_instruction* to_instruction(const repo_mapped_type& entry)
  {
    return const_cast<template_instruction*>(entry.message_.instruction());
  }

  fast_istream strm_;
  allocator* message_alloc_;
  repo_mapped_type* active_message_info_;
  bool force_reset_;
  decoder_presence_map* current_;


};


template <typename T>
class decoder_pmap_saver
{
  decoder_presence_map pmap_;
  decoder_presence_map* prev_pmap_;
  fast_decoder_core* core_;

public:
  decoder_pmap_saver(fast_decoder_core* core)
    : prev_pmap_(core->current_)
    , core_(core)
  {
    core_->current_ = &pmap_;
    core_->strm_.decode(pmap_);
  }

  ~decoder_pmap_saver()
  {
    core_->current_ = this->prev_pmap_;
  }

};

template <>
class decoder_pmap_saver<pmap_segment_size_zero>
{
public:
  decoder_pmap_saver(fast_decoder_core*)
  {
  }

  ~decoder_pmap_saver()
  {
  }

};



inline
fast_decoder_core::fast_decoder_core(allocator* alloc)
  : template_repo_base(this)
  , strm_(0)
  , message_alloc_(alloc)
  , active_message_info_(0)
  , force_reset_(false)
  , current_(0)
{
}

template <typename T>
inline void
fast_decoder_core::visit(const T& ext_ref)
{
  typedef typename T::type_category type_category;
  this->decode_field(ext_ref, type_category());
}

template <typename T, typename TypeCategory>
inline void
fast_decoder_core::decode_field(const T& ext_ref, TypeCategory)
{
  this->decode_field(ext_ref,
                     typename T::operator_category(),
                     TypeCategory());

}

template <typename T>
inline void
fast_decoder_core::decode_field(const T& ext_ref, split_decimal_type_tag)
{

  typename T::exponent_type exponent_ref = ext_ref.set_exponent();
  this->visit(exponent_ref);
  if (exponent_ref.present())
  {
    this->visit(ext_ref.set_mantissa());
  }
}

template <typename T>
inline void
fast_decoder_core::decode_field(const T& ext_ref, int_vector_type_tag)
{
  typename T::mref_type mref = ext_ref.set();

  uint32_t length=0;
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
inline void
fast_decoder_core::decode_field(const T& ext_ref, group_type_tag)
{
  // If a group field is optional, it will occupy a single bit in the presence map.
  // The contents of the group may appear in the stream iff the bit is set.
  if (ext_ref.optional())
  {
    if (!this->current_->is_next_bit_set()) {
      ext_ref.omit();
      return;
    }
  }

  decoder_pmap_saver<typename T::pmap_segment_size_type> saver(this);
  ext_ref.set().accept(*this);
}

template <typename T>
inline void
fast_decoder_core::decode_field(const T& ext_ref, sequence_type_tag)
{
  value_storage storage;

  typename T::length_type length = ext_ref.set_length(storage);
  this->visit(length);

  if (length.present()) {
    std::size_t len = length.get().value();
    ext_ref.set().resize(len);

    for (std::size_t i = 0; i < len; ++i)
    {
      this->visit(ext_ref[i]);
    }
  }
  else {
    ext_ref.omit();
  }
}

template <typename Message>
inline void
fast_decoder_core::decode_message(const message_mref& mref)
{
  typename Message::mref_type ref(mref);
  ref.accept(*this);
}

template <typename DescriptionsTuple>
inline void
fast_decoder_core::init(const DescriptionsTuple& tp)
{
  this->build(tp);
  active_message_info_ = this->unique_entry();
}

template <typename T, typename TypeCategory>
void fast_decoder_core::decode_field (const T& ext_ref,
                                      none_operator_tag,
                                      TypeCategory)
{
  fast_istream& stream = this->strm_;
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
void fast_decoder_core::decode_field(const T& ext_ref,
                                     constant_operator_tag,
                                     TypeCategory)
{
  decoder_presence_map& pmap= *this->current_;
  typename T::mref_type mref = ext_ref.set();

  if (ext_ref.optional()) {
    // An optional field with the constant operator will occupy a single bit. If the bit is set, the value
    // is the initial value in the instruction context. If the bit is not set, the value is considered absent.

    if (pmap.is_next_bit_set()) {
      mref.to_initial_value();
    }
    else {
      ext_ref.omit();
    }
  }
  else {
    // A field will not occupy any bit in the presence map if it is mandatory and has the constant operator.
    // mref.to_initial_value();
  }
  if (ext_ref.previous_value_shared())
      save_previous_value(mref);
}

template <typename T, typename TypeCategory>
void fast_decoder_core::decode_field(const T& ext_ref,
                                     copy_operator_tag,
                                     TypeCategory)
{
  fast_istream& stream = this->strm_;
  decoder_presence_map& pmap= *this->current_;
  typename T::mref_type mref = ext_ref.set ();

  if (pmap.is_next_bit_set()) {
    stream >> ext_ref;
    // A NULL indicates that the value is absent and the state of the previous value is set to empty
      save_previous_value(mref);
  } else {

    value_storage& previous = previous_value_of(mref);

    if (!previous.is_defined())
    {
      // if the previous value is undefined – the value of the field is the initial value
      // that also becomes the new previous value.

      // If the field has optional presence and no initial value, the field is considered
      // absent and the state of the previous value is changed to empty.
      mref.to_initial_value();
      save_previous_value(mref);

      if ( ext_ref.mandatory_without_initial_value()) {
        // Unless the field has optional presence, it is a dynamic error [ERR D5]
        // if the instruction context has no initial value.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D5"));
      }
    }
    else if (previous.is_empty()) {
      if (ext_ref.optional()) {
        // if the previous value is empty – the value of the field is empty.
        // If the field is optional the value is considered absent.
        mref.omit();
      }
      else {
        // It is a dynamic error [ERR D6] if the field is mandatory.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
      }

    }
    else {
      // if the previous value is assigned – the value of the field is the previous value.
      load_previous_value(mref);
    }
  }
}

template <typename T, typename TypeCategory>
void fast_decoder_core::decode_field(const T& ext_ref,
                                     increment_operator_tag,
                                     TypeCategory)
{
  fast_istream& stream = this->strm_;
  decoder_presence_map& pmap= *this->current_;
  typename T::mref_type mref = ext_ref.set ();

  if (pmap.is_next_bit_set()) {
    stream >> ext_ref;
    // A NULL indicates that the value is absent and the state of the previous value is set to empty
      save_previous_value(mref);
  } else {

    value_storage& previous = previous_value_of(mref);

    if (!previous.is_defined())
    {
      // if the previous value is undefined – the value of the field is the initial value
      // that also becomes the new previous value.

      // If the field has optional presence and no initial value, the field is considered
      // absent and the state of the previous value is changed to empty.
      mref.to_initial_value();
      save_previous_value(mref);

      if (ext_ref.mandatory_without_initial_value()) {
        // Unless the field has optional presence, it is a dynamic error [ERR D5]
        // if the instruction context has no initial value.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D5"));
      }
    }
    else if (previous.is_empty()) {
      // if the previous value is empty – the value of the field is empty.
      if (ext_ref.optional()) {
        // If the field is optional the value is considered absent.
        mref.omit();
      }
      else {
        // It is a dynamic error [ERR D6] if the field is mandatory.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
      }

    }
    else {
      typedef typename T::mref_type::value_type value_type;
      previous.set<value_type>(previous.get<value_type>() + 1 );
      // if the previous value is assigned – the value of the field is the previous value.
      load_previous_value(mref);
    }
  }
}

template <typename T, typename TypeCategory>
void fast_decoder_core::decode_field(const T& ext_ref,
                                     default_operator_tag,
                                     TypeCategory)
{
  fast_istream& stream = this->strm_;
  decoder_presence_map& pmap= *this->current_;
  typename T::mref_type mref = ext_ref.set ();

  // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.
  // Optional integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream in a nullable representation.

  if (pmap.is_next_bit_set()) {
    stream >> ext_ref;
    //  A NULL indicates that the value is absent and the state of the previous value is left unchanged.
    if (!ext_ref.present())
      return;
  }
  else {
    // If the field has optional presence and no initial value, the field is considered absent
    // when there is no value in the stream.

    //  The default operator specifies that the value of a field is either present in the stream
    //  or it will be the initial value.
    mref.to_initial_value();
  }

  if (ext_ref.previous_value_shared())
    save_previous_value(mref);
}

template <typename T>
void fast_decoder_core::decode_field(const T& ext_ref,
                                     delta_operator_tag,
                                     integer_type_tag)
{
  fast_istream& stream = this->strm_;
  typename T::mref_type mref = ext_ref.set ();
  typedef typename T::mref_type::value_type int_type;

  int64_t d;
  if (stream.decode(d, ext_ref.nullable() )) {

    value_storage bv = delta_base_value_of( mref );
    typename T::mref_type tmp(0, &bv, 0);

    // check_overflow(tmp.value(), d, mref.instruction(), stream);
    mref.as( static_cast<int_type>(tmp.value()+d) );

    save_previous_value(mref);
  }
  else {
    //  If the field has optional presence, the delta value can be NULL. In that case the value of the field is considered absent.
    mref.omit();
  }
}

template <typename T>
void fast_decoder_core::decode_field(const T& ext_ref,
                                     delta_operator_tag,
                                     string_type_tag)
{
  fast_istream& stream = this->strm_;
  typename T::mref_type mref = ext_ref.set();
  // The delta value is represented as a Signed Integer subtraction length followed by an ASCII String.
  // If the delta is nullable, the subtraction length is nullable. A NULL delta is represented as a
  // NULL subtraction length. The string part is present in the stream iff the subtraction length is not NULL.
  int32_t substraction_length;
  if (stream.decode(substraction_length, ext_ref.nullable() )) {
    // It is a dynamic error [ERR D7] if the subtraction length is larger than the
    // number of characters in the base value, or if it does not fall in the value range of an int32.
    int32_t sub_len = substraction_length >= 0 ? substraction_length : ~substraction_length;
    const value_storage& base_value = delta_base_value_of(mref);

    if ( sub_len > static_cast<int32_t>(base_value.array_length()))
      BOOST_THROW_EXCEPTION(fast_dynamic_error("D7"));

    uint32_t delta_len;
    const typename T::mref_type::value_type* delta_str=0;
    stream.decode(delta_str, delta_len, mref.instruction(), false_type());

    this->apply_string_delta(mref,
                             base_value,
                             substraction_length,
                             delta_str,
                             delta_len);
    save_previous_value(mref);
  }
  else {
    mref.omit();
  }
}

template <typename T>
void fast_decoder_core::decode_field(const T& ext_ref,
                                     delta_operator_tag,
                                     decimal_type_tag)
{
  fast_istream& stream = this->strm_;
  decimal_mref mref = ext_ref.set();
  stream >> ext_ref;
  if (!ext_ref.optional() || mref.present()) {
    value_storage bv = delta_base_value_of(mref);

    // check_overflow(bv.of_decimal.mantissa_, mref.mantissa(), mref.instruction(), stream);
    // check_overflow(bv.of_decimal.exponent_, mref.exponent(), mref.instruction(), stream);
    mref.set_mantissa( bv.of_decimal.mantissa_ + mref.mantissa() );
    mref.set_exponent( bv.of_decimal.exponent_ + mref.exponent() );
    // if (mref.exponent() > 63 || mref.exponent() < -63 )
    //   BOOST_THROW_EXCEPTION(fast_reportable_error("R1"));
    //
    save_previous_value(mref);
  }
}

template <typename T>
void fast_decoder_core::decode_field(const T& ext_ref,
                                     tail_operator_tag,
                                     string_type_tag)
{
  fast_istream& stream = this->strm_;
  decoder_presence_map& pmap= *this->current_;

  typename T::mref_type mref = ext_ref.set();

  if (pmap.is_next_bit_set()) {

    uint32_t len;
    const typename T::mref_type::value_type* str;
    if (stream.decode(str, len, mref.instruction(), ext_ref.nullable()) ) {
      const value_storage& base_value (tail_base_value_of(mref));
      this->apply_string_delta(mref,
                               base_value,
                               std::min<int>(len, base_value.array_length()),
                               str,
                               len);
    }
    else {
      //If the field has optional presence, the tail value can be NULL.
      // In that case the value of the field is considered absent.
      mref.omit();
    }
  }
  else {
    // If the tail value is not present in the stream, the value of the field depends
    // on the state of the previous value in the following way:

    value_storage& prev = previous_value_of(mref);

    if (!prev.is_defined()) {
      //  * undefined – the value of the field is the initial value that also becomes the new previous value.

      // If the field has optional presence and no initial value, the field is considered absent and the state of the previous value is changed to empty.
      mref.to_initial_value();

      if (ext_ref.mandatory_without_initial_value()) {
        // Unless the field has optional presence, it is a dynamic error [ERR D6] if the instruction context has no initial value.
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));
      }
    }
    else if (prev.is_empty()) {
      //  * empty – the value of the field is empty. If the field is optional the value is considered absent.
      //            It is a dynamic error [ERR D7] if the field is mandatory.
      if (ext_ref.optional())
        mref.omit();
      else
        BOOST_THROW_EXCEPTION(fast_dynamic_error("D7"));
    }
    else {
      // * assigned – the value of the field is the previous value.
      load_previous_value(mref);
      return;
    }
  }
  save_previous_value(mref);
}

}   /* coder */



} /* mfast */



#endif /* end of include guard: FAST_DECODER_CORE_H_51998563 */
