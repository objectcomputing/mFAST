#ifndef DECODER_FIELD_OPERATOR_V2_H_D94E86CA
#define DECODER_FIELD_OPERATOR_V2_H_D94E86CA

#include "../common/codec_helper.h"
#include "../decoder/fast_istream.h"
#include "../decoder/decoder_presence_map.h"
// #include "../decoder/check_overflow.h"
#include "fast_istream_extractor.h"

namespace mfast
{
  namespace coder
  {

    class decoder_function
      : public mfast::detail::codec_helper
    {
    public:

      template <typename T>
      void decode (const T&              ext_ref,
                   fast_istream&         stream,
                   decoder_presence_map& pmap) const
      {
        this->decode(ext_ref, stream, pmap, typename T::operator_category(), typename T::type_category());
      }

      template <typename T, typename TypeCategory>
      void decode (const T&      ext_ref,
                   fast_istream& stream,
                   decoder_presence_map& /*pmap*/,
                   none_operator_tag,
                   TypeCategory) const
      {
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
      void decode(const T&              ext_ref,
                  fast_istream& /* stream */,
                  decoder_presence_map& pmap,
                  constant_operator_tag,
                  TypeCategory) const
      {
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
      void decode(const T&              ext_ref,
                  fast_istream&         stream,
                  decoder_presence_map& pmap,
                  copy_operator_tag,
                  TypeCategory) const
      {
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
      void decode(const T&              ext_ref,
                  fast_istream&         stream,
                  decoder_presence_map& pmap,
                  increment_operator_tag,
                  TypeCategory) const
      {
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
      void decode(const T&              ext_ref,
                  fast_istream&         stream,
                  decoder_presence_map& pmap,
                  default_operator_tag,
                  TypeCategory) const
      {
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
      void decode(const T&      ext_ref,
                  fast_istream& stream,
                  decoder_presence_map& /* pmap */,
                  delta_operator_tag,
                  integer_type_tag) const
      {
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
      void decode(const T&      ext_ref,
                  fast_istream& stream,
                  decoder_presence_map& /* pmap */,
                  delta_operator_tag,
                  string_type_tag) const
      {
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
      void decode(const T&   ext_ref,
                  fast_istream&         stream,
                  decoder_presence_map& /*pmap*/,
                  delta_operator_tag,
                  decimal_type_tag) const
      {
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
      void decode(const T&  ext_ref,
                  fast_istream&         stream,
                  decoder_presence_map& pmap,
                  tail_operator_tag,
                  string_type_tag) const
      {
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

    };


  } /* coder */

} /* mfast */


#endif /* end of include guard: DECODER_FIELD_OPERATOR_V2_H_D94E86CA */
