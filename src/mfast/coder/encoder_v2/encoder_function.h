#ifndef ENCODER_FUNCTION_H_CF2EDCBA
#define ENCODER_FUNCTION_H_CF2EDCBA

#include "mfast/ext_ref.h"
#include "fast_ostream_inserter.h"

namespace mfast
{
  namespace coder
  {


    template <typename T>
    inline bool is_same(const int_cref<T>& v, const value_storage& prev)
    {
      return v.absent() == prev.is_empty() && v.value() == prev.get<T>();
    }

    inline bool is_same(const exponent_cref& v, const value_storage& prev)
    {
      return v.absent() == prev.is_empty() && v.value() == prev.of_decimal.exponent_;
    }

    inline bool is_same (const decimal_cref& v, const value_storage& prev)
    {
      return v.absent() == prev.is_empty() && v.mantissa() == prev.of_decimal.mantissa_ && v.exponent() == prev.of_decimal.exponent_;
    }

    template <typename T>
    inline bool is_same (const vector_cref<T>& v, const value_storage& prev)
    {
      return v.size() == prev.of_array.len_-1 && memcmp(v.data(),prev.of_array.content_, v.size()*sizeof(T)) == 0;
    }

    class encoder_function
      : public mfast::detail::codec_helper
    {
    public:

      template <typename T>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& pmap) const
      {
        this->encode(ext_ref, stream, pmap, typename T::operator_category(), typename T::type_category());
      }

      template <typename T, typename TypeCategory>
      void encode (const T&  ext_ref,
                   fast_ostream& stream,
                   encoder_presence_map  & /* pmap */,
                   none_operator_tag,
                   TypeCategory) const
      {
        stream << ext_ref;

        // Fast Specification 1.1, page 22
        //
        // If a field is mandatory and has no field operator, it will not occupy any
        // bit in the presence map and its value must always appear in the stream.
        //
        // If a field is optional and has no field operator, it is encoded with a
        // nullable representation and the NULL is used to represent absence of a
        // value. It will not occupy any bits in the presence map.
        if (ext_ref.previous_value_shared())
          stream.save_previous_value(ext_ref.get());
      }

      template <typename T, typename TypeCategory>
      void encode(const T&              ext_ref,
                  fast_ostream& /* stream */,
                  encoder_presence_map& pmap,
                  constant_operator_tag,
                  TypeCategory) const
      {
        typename T::cref_type cref = ext_ref.get();

        if (ext_ref.optional()) {
          // A field will not occupy any bit in the presence map if it is mandatory and has the constant operator.
          // An optional field with the constant operator will occupy a single bit. If the bit is set, the value
          // is the initial value in the instruction context. If the bit is not set, the value is considered absent.
          pmap.set_next_bit(cref.present());
        }

        if (ext_ref.previous_value_shared())
          save_previous_value(cref);
      }

      template <typename T, typename TypeCategory>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& pmap,
                  copy_operator_tag,
                  TypeCategory) const
      {
        typename T::cref_type cref = ext_ref.get();

        value_storage previous = previous_value_of(cref);
        stream.save_previous_value(cref);

        if (!previous.is_defined())
        {
          // if the previous value is undefined – the value of the field is the initial value
          // that also becomes the new previous value.
          // If the field has optional presence and no initial value, the field is considered
          // absent and the state of the previous value is changed to empty.
          if (cref.is_initial_value()) {

            pmap.set_next_bit(false);
            return;
          }
        }
        else if (previous.is_empty()) {
          // if the previous value is empty – the value of the field is empty.
          // If the field is optional the value is considered absent.
          if (!ext_ref.present()) {
            pmap.set_next_bit(false);
            return;
          }
          else if (!ext_ref.optional() ) {
            // It is a dynamic error [ERR D6] if the field is mandatory.
            BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));

            // We need to handle this case because the previous value may have been
            // modified by another instruction with the same key and that intruction
            // has optional presence.
          }
        }
        else if ( is_same (cref, previous) ) {
          pmap.set_next_bit(false);
          return;
        }

        pmap.set_next_bit(true);
        stream << ext_ref;
      }

      template <typename T, typename TypeCategory>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& pmap,
                  increment_operator_tag,
                  TypeCategory) const
      {
        typename T::cref_type cref = ext_ref.get();

        value_storage previous = previous_value_of(cref);
        stream.save_previous_value(cref);

        if (!previous.is_defined())
        {
          // if the previous value is undefined – the value of the field is the initial value
          // that also becomes the new previous value.
          // If the field has optional presence and no initial value, the field is considered
          // absent and the state of the previous value is changed to empty.
          if (cref.is_initial_value()) {

            pmap.set_next_bit(false);
            return;
          }
        }
        else if (previous.is_empty()) {
          // if the previous value is empty – the value of the field is empty.
          // If the field is optional the value is considered absent.
          if (!ext_ref.present()) {
            pmap.set_next_bit(false);
            return;
          }
          else if (!ext_ref.optional() ) {
            // It is a dynamic error [ERR D6] if the field is mandatory.
            BOOST_THROW_EXCEPTION(fast_dynamic_error("D6"));

            // We need to handle this case because the previous value may have been
            // modified by another instruction with the same key and that intruction
            // has optional presence.
          }
        }
        else if (cref.value() == previous.get<const typename T::cref_type::value_type>() + 1) {
          pmap.set_next_bit(false);
          return;
        }

        pmap.set_next_bit(true);
        stream << ext_ref;
      }

      template <typename T, typename TypeCategory>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& pmap,
                  default_operator_tag,
                  TypeCategory) const
      {
        typename T::cref_type cref = ext_ref.get();

        // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.
        // Optional integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream in a nullable representation.


        //  The default operator specifies that the value of a field is either present in the stream
        //  or it will be the initial value.

        // If the field has optional presence and no initial value, the field is considered absent
        // when there is no value in the stream.

        if (cref.is_initial_value()) {
          pmap.set_next_bit(false);
          if (ext_ref.previous_value_shared())
            stream.save_previous_value(cref);
          return;
        }

        pmap.set_next_bit(true);
        if (!ext_ref.present() ) {
          //  A NULL indicates that the value is absent and the state of the previous value is left unchanged.
          stream.encode_null();
        }
        else {
          stream << ext_ref;
          if (ext_ref.previous_value_shared())
            save_previous_value(cref);
        }
      }

      template <typename T>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& /*pmap*/,
                  delta_operator_tag,
                  integer_type_tag) const
      {
        typename T::cref_type cref = ext_ref.get();

        if (!ext_ref.present()) {
          //  If the field has optional presence, the delta value can be NULL. In that case the value of the field is considered absent.
          stream.encode_null();
        }
        else {
          value_storage bv = delta_base_value_of( cref );
          typename T::cref_type base(&bv, 0);

          int64_t delta = static_cast<int64_t>(cref.value() - base.value());

          stream.encode(delta, false, ext_ref.nullable());
          stream.save_previous_value(cref);
        }

      }

      template <typename T>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& /*pmap*/,
                  delta_operator_tag,
                  string_type_tag) const
      {
        typename T::cref_type cref = ext_ref.get();

        if (!ext_ref.present()) {
          stream.encode_null();
          return;
        }

        const value_storage& prev = delta_base_value_of(cref);

        typename T::cref_type prev_cref(&prev, cref.instruction());
        typedef typename T::cref_type::const_iterator const_iterator;
        typedef typename T::cref_type::const_reverse_iterator const_reverse_iterator;
        typedef typename std::iterator_traits<const_iterator>::difference_type difference_type;

        std::pair<const_iterator, const_iterator> common_prefix_positions
          = std::mismatch(cref.begin(), cref.end(), prev_cref.begin());

        std::pair<const_reverse_iterator, const_reverse_iterator> common_suffix_positions
          = std::mismatch(cref.rbegin(), cref.rend(), prev_cref.rbegin());

        int32_t substraction_len;
        const_iterator delta_iterator;
        uint32_t delta_len;

        difference_type common_prefix_delta_len = cref.end() - common_prefix_positions.first;
        difference_type common_suffix_delta_len = cref.rend() - common_suffix_positions.first;

        if ( common_prefix_delta_len <= common_suffix_delta_len ) {
          substraction_len = static_cast<int32_t>(prev_cref.end() - common_prefix_positions.second);
          delta_iterator = common_prefix_positions.first;
          delta_len = static_cast<uint32_t>(common_prefix_delta_len);
        }
        else {
          // Characters are removed from the front when the subtraction length is negative.
          // The subtraction length uses an excess-1 encoding: if the value is negative when decoding,
          // it is incremented by one to get the number of characters to subtract. This makes it possible
          // to encode negative zero as -1,
          substraction_len = static_cast<int32_t>(~(prev_cref.rend() - common_suffix_positions.second));
          delta_iterator = cref.begin();
          delta_len = static_cast<uint32_t>(common_suffix_delta_len);
        }

        stream.encode(substraction_len, false, ext_ref.nullable());
        stream.encode(delta_iterator, delta_len, cref.instruction(), false_type());

        stream.save_previous_value(cref);
      }

      template <typename T>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& /*pmap*/,
                  delta_operator_tag,
                  decimal_type_tag) const
      {

        if (ext_ref.present()) {
          decimal_cref cref = ext_ref.get();

          value_storage bv = delta_base_value_of(cref);

          value_storage delta_storage;
          delta_storage.of_decimal.exponent_ = cref.exponent() - bv.of_decimal.exponent_;
          delta_storage.of_decimal.mantissa_ = cref.mantissa() - bv.of_decimal.mantissa_;

          decimal_cref delta(&delta_storage, cref.instruction());
          stream << T(delta);

          stream.save_previous_value(cref);
        }
        else {
          stream.encode_null();
        }
      }

      template <typename T>
      void encode(const T&              ext_ref,
                  fast_ostream&         stream,
                  encoder_presence_map& pmap,
                  tail_operator_tag,
                  string_type_tag) const
      {
        typename T::cref_type cref = ext_ref.get();

        value_storage& prev = previous_value_of(cref);

        if (is_same(cref, tail_base_value_of(cref))) {
          pmap.set_next_bit(false);
        }
        else if (!ext_ref.present()) {
          if (prev.is_defined() && prev.is_empty()) {
            pmap.set_next_bit(false);
          }
          else {
            pmap.set_next_bit(true);
            stream.encode_null();
          }
        }
        else {
          pmap.set_next_bit(true);

          uint32_t tail_len;
          typedef typename T::cref_type::const_iterator const_iterator;

          const_iterator tail_itr;

          value_storage base = tail_base_value_of(cref);
          typename T::cref_type base_cref(&base, cref.instruction());

          if (cref.size() == base_cref.size()) {


            std::pair<const_iterator, const_iterator> positions
              = std::mismatch(cref.begin(), cref.end(), base_cref.begin());

            tail_itr = positions.first;
            tail_len = static_cast<uint32_t>(cref.end()-positions.first);
          }
          else {
            tail_itr = cref.begin();
            tail_len = static_cast<uint32_t>(cref.size());
          }
          stream.encode(tail_itr,
                        tail_len,
                        cref.instruction(),
                        ext_ref.nullable());
        }
        stream.save_previous_value(cref);

      }

    };

  } /* coder */

} /* mfast */


#endif /* end of include guard: ENCODER_FUNCTION_H_CF2EDCBA */
