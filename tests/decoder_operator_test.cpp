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
#include <mfast/fast_istream.h>
#include <mfast/codec_helper.h>
#include <mfast/int_ref.h>
#include <mfast/decoder_field_operator.h>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>
#include "debug_allocator.h"
#include <stdexcept>

using namespace mfast;



BOOST_AUTO_TEST_SUITE( decoder_operator_test_suit )


bool str_equal(const char* lhs, const char* rhs)
{
  return std::strcmp(lhs, rhs) ==0;
}


BOOST_AUTO_TEST_CASE(operator_none_test)
{
  malloc_allocator allocator;
  value_storage_t storage;

  {
    uint64_field_instruction inst(operator_none,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);

    // If a field is optional and has no field operator, it is encoded with a
    // nullable representation and the NULL is used to represent absence of a
    // value. It will not occupy any bits in the presence map.

    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we don't consume any bit in presence map
    BOOST_CHECK_EQUAL(old_mask, pmap.mask());

    BOOST_CHECK(ref.absent());

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());
  }
  {
    uint64_field_instruction inst(operator_none,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);

    // If a field is optional and has no field operator, it is encoded with a
    // nullable representation and the NULL is used to represent absence of a
    // value. It will not occupy any bits in the presence map.

    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we don't consume any bit in presence map
    BOOST_CHECK_EQUAL(old_mask, pmap.mask());

    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.value(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK_EQUAL(prev.value(), 0);
    BOOST_CHECK(prev.present());
  }
}

BOOST_AUTO_TEST_CASE(operator_constant_test)
{

  malloc_allocator allocator;
  value_storage_t storage;

  {
    // An optional field with the constant operator will occupy a single bit. If the bit is set, the value
    // is the initial value in the instruction context. If the bit is not set, the value is considered absent.

    uint64_field_instruction inst(operator_constant,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    {
      // testing when the presence bit is set
      char data[] = "\xC0\x80";
      fast_istream strm(data, 2);
      presence_map pmap;

      strm.decode(pmap);
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);
      uint64_t old_mask = pmap.mask();

      decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

      // make sure we consume 1 bit in presence map
      BOOST_CHECK_EQUAL(old_mask >> 1, pmap.mask());

      BOOST_CHECK_EQUAL(ref.value(), UINT64_MAX);
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);

      uint64_cref prev(&inst.prev_value(), &inst);
      // check the previous value is properly set
      BOOST_CHECK(inst.prev_value().is_defined());
      BOOST_CHECK(prev.present());
      BOOST_CHECK_EQUAL(prev.value(), UINT64_MAX);
    }

    {
      // testing when the presence bit is not set
      char data[] = "\x80\x80";
      fast_istream strm(data, 2);
      presence_map pmap;

      strm.decode(pmap);
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);
      uint64_t old_mask = pmap.mask();

      decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

      // make sure we consume 1 bit in presence map
      BOOST_CHECK_EQUAL(old_mask >> 1, pmap.mask());

      BOOST_CHECK(ref.absent());
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);

      uint64_cref prev(&inst.prev_value(), &inst);
      // check the previous value is properly set
      BOOST_CHECK(inst.prev_value().is_defined());
      BOOST_CHECK(prev.absent());
    }

  }
  {
      // A field will not occupy any bit in the presence map if it is mandatory and has the constant operator.

    uint64_field_instruction inst(operator_constant,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    BOOST_CHECK(!ref.is_optional());

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we don't consume any bit in presence map
    BOOST_CHECK_EQUAL(old_mask, pmap.mask());

    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.value(), UINT64_MAX);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value(), UINT64_MAX);
  }
}

BOOST_AUTO_TEST_CASE(operator_default_test)
{

  malloc_allocator allocator;
  value_storage_t storage;

  {
    uint64_field_instruction inst(operator_default,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.


    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.value(), 0);
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value(), 0);
  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    // value is not present in the stream, the value is set to initial value
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.value(), UINT64_MAX);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value(), UINT64_MAX);
  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;

    // Optional integer, decimal, string and byte vector fields – one bit.
    // If set, the value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous
    // value  is left unchanged.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    // value is not present in the stream, the value is set to initial value
    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(!inst.prev_value().is_defined());
  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;

    // The default operator specifies that the value of a field is either present in the stream or it will be the initial value.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    // value is not present in the stream, the value is set to initial value
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.value(), UINT64_MAX);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value(), UINT64_MAX);
  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0);
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

     // If the field has optional presence and no initial value, the field is considered absent when there is no value in the stream.
    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());
  }
}

BOOST_AUTO_TEST_CASE(operator_copy_test)
{
  malloc_allocator allocator;
  value_storage_t storage;

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.


    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.value(), 0);
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value(), 0);
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.value(), UINT64_MAX);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_mref prev(&allocator, &inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value(), UINT64_MAX);

    /// set previous value to a different value
    prev.as(5);

    old_mask = pmap.mask();
    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 5);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 5);

    /// set previous value to empty
    inst.prev_value().present(false);

    old_mask = pmap.mask();


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * empty – the value of the field is empty. If the field is optional the value is considered absent. It is a dynamic error [ERR D6] if the field is mandatory.
    BOOST_CHECK_THROW(decoder_operators[inst.field_operator()]->decode(ref, strm, pmap), mfast::fast_error);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – one bit.
    // If set, the value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – one bit.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty
    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , UINT64_MAX);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_mref prev(&allocator, &inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , UINT64_MAX);

    /// set previous value to a different value
    prev.as(5);

    old_mask = pmap.mask();
    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 5);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 5);

    /// set previous value to empty
    inst.prev_value().present(false);

    old_mask = pmap.mask();


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * empty – the value of the field is empty. If the field is optional the value is considered absent.
   decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());
  }

  { // testing no initial value
      uint64_field_instruction inst(operator_copy,
                                    presence_optional,
                                    1,
                                    "test_uint64","",
                                    0);
        inst.construct_value(storage, &allocator);


      uint64_mref ref(&allocator, &storage, &inst);

      char data[] = "\x80\x80";
      fast_istream strm(data, 2);
      presence_map pmap;
      // Optional integer, decimal, string and byte vector fields – one bit.
      // A NULL indicates that the value is absent and the state of the previous value is set to empty
      strm.decode(pmap);
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);
      uint64_t old_mask = pmap.mask();

      decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

      // make sure we  consume 1 bit in presence map
      BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());


      // When the value is not present in the stream there are three cases depending on the state of the previous value:
      // * undefined – If the field has optional presence and no initial value, the field is considered absent and the state of the previous value is changed to empty.
      BOOST_CHECK(ref.absent());
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);

      uint64_mref prev(&allocator, &inst.prev_value(), &inst);
      // check the previous value is properly set
      BOOST_CHECK(inst.prev_value().is_defined());
      BOOST_CHECK(prev.absent());
  }
}

BOOST_AUTO_TEST_CASE(operator_increment_test)
{
  malloc_allocator allocator;
  value_storage_t storage;

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.


    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 0);
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 0);
  }

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , UINT64_MAX);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_mref prev(&allocator, &inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , UINT64_MAX);

    /// set previous value to a different value
    prev.as(5);

    old_mask = pmap.mask();
    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value incremented by one. The incremented value also becomes the new previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 6);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 6);

    /// set previous value to empty
    inst.prev_value().present(false);

    old_mask = pmap.mask();


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * empty – the value of the field is empty. If the field is optional the value is considered absent. It is a dynamic error [ERR D6] if the field is mandatory.
    BOOST_CHECK_THROW(decoder_operators[inst.field_operator()]->decode(ref, strm, pmap), mfast::fast_error);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());
  }

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – one bit.
    // If set, the value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());
  }

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – one bit.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty
    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , UINT64_MAX);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    uint64_mref prev(&allocator, &inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , UINT64_MAX);

    /// set previous value to a different value
    prev.as(5);

    old_mask = pmap.mask();
    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value incremented by one. The incremented value also becomes the new previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 6);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 6);

    /// set previous value to empty
    inst.prev_value().present(false);

    old_mask = pmap.mask();


    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * empty – the value of the field is empty. If the field is optional the value is considered absent.
   decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // make sure we  consume 1 bit in presence map
    BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());

    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());
  }

  { // testing no initial value
      uint64_field_instruction inst(operator_increment,
                                    presence_optional,
                                    1,
                                    "test_uint64","",
                                    0);
        inst.construct_value(storage, &allocator);


      uint64_mref ref(&allocator, &storage, &inst);

      char data[] = "\x80\x80";
      fast_istream strm(data, 2);
      presence_map pmap;
      // Optional integer, decimal, string and byte vector fields – one bit.
      // A NULL indicates that the value is absent and the state of the previous value is set to empty
      strm.decode(pmap);
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);
      uint64_t old_mask = pmap.mask();

      decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

      // make sure we  consume 1 bit in presence map
      BOOST_CHECK_EQUAL(old_mask >>1 , pmap.mask());


      // When the value is not present in the stream there are three cases depending on the state of the previous value:
      // * undefined – If the field has optional presence and no initial value, the field is considered absent and the state of the previous value is changed to empty.
      BOOST_CHECK(ref.absent());
      BOOST_CHECK_EQUAL(strm.in_avail(), 1);

      uint64_mref prev(&allocator, &inst.prev_value(), &inst);
      // check the previous value is properly set
      BOOST_CHECK(inst.prev_value().is_defined());
      BOOST_CHECK(prev.absent());
  }
}

BOOST_AUTO_TEST_CASE(operator_delta_integer_test)
{
  malloc_allocator allocator;
  value_storage_t storage;

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(5ULL));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x82";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.


    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 7);
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 7);
  }

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0); // no initial value
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x82";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.


    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 2);
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 2);
  }

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(5ULL));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x82";
    fast_istream strm(data, 2);
    presence_map pmap;

    //Optional integer, decimal, string and byte vector fields – no bit.
    // The delta appears in the stream in a nullable representation. A NULL indicates that the delta is absent.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref , 6);
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.value() , 6);
  }

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  nullable<uint64_t>(5ULL));
    inst.construct_value(storage, &allocator);


    uint64_mref ref(&allocator, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;

    //Optional integer, decimal, string and byte vector fields – no bit.
    // The delta appears in the stream in a nullable representation. A NULL indicates that the delta is absent.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    //  If the field has optional presence, the delta value can be NULL. In that case the value of the field is considered absent.
    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    uint64_cref prev(&inst.prev_value(), &inst);
    // Note that the previous value is not set to empty but is left untouched if the value is absent.
    BOOST_CHECK(!inst.prev_value().is_defined());
  }

}

BOOST_AUTO_TEST_CASE(operator_delta_decimal_test)
{
  malloc_allocator allocator;
  value_storage_t storage;

  {
    decimal_field_instruction inst(operator_delta,
                                   presence_mandatory,
                                   1,
                                   "test_decimal","",
                                   0,
                                   nullable_decimal(12,1)); //  initial

    inst.construct_value(storage, &allocator);
    decimal_mref ref(&allocator, &storage, &inst);
    char data[] = "\xC0\x82\x83";
    fast_istream strm(data, 3);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 2);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.mantissa(), 15); // 12 (base) + 3 (delta)
    BOOST_CHECK_EQUAL(ref.exponent(), 3); // 1 (base) + 2 (delta)

    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    decimal_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.mantissa(), 15); // 12 (base) + 3 (delta)
    BOOST_CHECK_EQUAL(prev.exponent(), 3); // 1 (base) + 2 (delta)

  }
  {
    decimal_field_instruction inst(operator_delta,
                                   presence_mandatory,
                                   1,
                                   "test_decimal","",
                                   0); // no initial value

    inst.construct_value(storage, &allocator);
    decimal_mref ref(&allocator, &storage, &inst);
    char data[] = "\xC0\x82\x83";
    fast_istream strm(data, 3);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 2);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.mantissa(), 3); // 0 (base) + 3 (delta)
    BOOST_CHECK_EQUAL(ref.exponent(),2); // 0 (base) + 2 (delta)

    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    decimal_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_EQUAL(prev.mantissa(), 3);
    BOOST_CHECK_EQUAL(prev.exponent(), 2);

  }

  {
    decimal_field_instruction inst(operator_delta,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0); // no initial value

    inst.construct_value(storage, &allocator);
    decimal_mref ref(&allocator, &storage, &inst);
    char data[] = "\xC0\x80\x83";
    fast_istream strm(data, 3);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 2);
    uint64_t old_mask = pmap.mask();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    BOOST_CHECK(ref.absent());


    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    decimal_cref prev(&inst.prev_value(), &inst);
    // Note that the previous value is not set to empty but is left untouched if the value is absent.
    BOOST_CHECK(!inst.prev_value().is_defined());
  }
}

BOOST_AUTO_TEST_CASE(operator_delta_ascii_test)
{
  debug_allocator alloc;
  value_storage_t storage;

  { // testing mandatory field with initial value
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_delta,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x86\x76\x61\x6C\x75\xE5";
    fast_istream strm(data, 7);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 6);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_value"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    ascii_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_value"));

    inst.destruct_value(storage, &alloc);
  }

  { // testing mandatory field without initial value
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_delta,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 0);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x86\x76\x61\x6C\x75\xE5";
    fast_istream strm(data, 7);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 6);
    uint64_t old_mask = pmap.mask();


    BOOST_CHECK_THROW(decoder_operators[inst.field_operator()]->decode(ref, strm, pmap),
                      mfast::fast_error);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with NULL substraction in the stream
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_delta,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // If the field has optional presence, the delta value can be NULL.
    // In that case the value of the field is considered absent.
    BOOST_CHECK(ref.absent());

    ascii_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set

    // Optional integer, decimal, string and byte vector fields – no bit.
    // The delta appears in the stream in a nullable representation.
    // A NULL indicates that the delta is absent.
    // Note that the previous value is not set to empty but is left untouched if the value is absent.
    BOOST_CHECK(!inst.prev_value().is_defined());

    inst.destruct_value(storage, &alloc);
  }
  { // testing optional field with positive substraction in the stream
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_delta,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x83\x41\x42\x43\xC4";
    fast_istream strm(data, 6);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 5);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_striABCD"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    ascii_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_striABCD"));

    inst.destruct_value(storage, &alloc);
  }
}

BOOST_AUTO_TEST_CASE(operator_delta_unicode_test)
{
  debug_allocator alloc;
  value_storage_t storage;

  { // testing mandatory field with initial value
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    unicode_field_instruction inst(operator_delta,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    unicode_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x86\x85\x76\x61\x6C\x75\x65";
    fast_istream strm(data, 8);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 7);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_value"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    unicode_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_value"));

    inst.destruct_value(storage, &alloc);
  }

  { // testing mandatory field without initial value
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    unicode_field_instruction inst(operator_delta,
                                 presence_mandatory,
                                 1,
                                 "test_unicode","",
                                 0);

    inst.construct_value(storage, &alloc);

    unicode_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x86\x85\x76\x61\x6C\x75\x65";
    fast_istream strm(data, 8);
    presence_map pmap;
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 7);
    uint64_t old_mask = pmap.mask();


    BOOST_CHECK_THROW(decoder_operators[inst.field_operator()]->decode(ref, strm, pmap),
                      mfast::fast_error);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with NULL substraction in the stream
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    unicode_field_instruction inst(operator_delta,
                                 presence_optional,
                                 1,
                                 "test_unicode","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    unicode_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // If the field has optional presence, the delta value can be NULL.
    // In that case the value of the field is considered absent.
    BOOST_CHECK(ref.absent());

    unicode_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set

    // Optional integer, decimal, string and byte vector fields – no bit.
    // The delta appears in the stream in a nullable representation.
    // A NULL indicates that the delta is absent.
    // Note that the previous value is not set to empty but is left untouched if the value is absent.
    BOOST_CHECK(!inst.prev_value().is_defined());

    inst.destruct_value(storage, &alloc);
  }
  { // testing optional field with positive substraction in the stream
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    unicode_field_instruction inst(operator_delta,
                                 presence_optional,
                                 1,
                                 "test_unicode","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    unicode_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x83\x84\x41\x42\x43\x44";
    fast_istream strm(data, 7);
    presence_map pmap;
    // Optional integer, decimal, string and byte vector fields – no bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 6);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we  don't consume bit in presence map
    BOOST_CHECK_EQUAL(old_mask  , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_striABCD"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    unicode_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_striABCD"));

    inst.destruct_value(storage, &alloc);
  }
}

BOOST_AUTO_TEST_CASE(operator_tail_ascii_test)
{
  debug_allocator alloc;
  value_storage_t storage;

  { // testing mandatory field with initial value
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_tail,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x76\x61\x6C\x75\xE5";
    fast_istream strm(data, 6);
    presence_map pmap;
    // Mandatory string and byte vector fields – one bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 5);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we consume one bit in presence map
    BOOST_CHECK_EQUAL(old_mask >> 1 , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_svalue"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    ascii_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_svalue"));

    inst.destruct_value(storage, &alloc);
  }

  { // testing mandatory field with initial value while tail value not in the stream
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_tail,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory string and byte vector fields – one bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we consume one bit in presence map
    BOOST_CHECK_EQUAL(old_mask >> 1 , pmap.mask());

    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
   //  undefined – the value of the field is the initial value that also becomes the new previous value..
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_string"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    ascii_string_mref prev(&alloc, &inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_string"));

    // change the previous value to "ABCDE" so we can verified the case with defined previous value
    prev = "ABCDE";

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
   //  assigned – the value of the field is the previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("ABCDE"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("ABCDE"));

    inst.destruct_value(storage, &alloc);

  }
  { // testing mandatory field without initial value while tail value not in the stream

    ascii_field_instruction inst(operator_tail,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 0);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Mandatory string and byte vector fields – one bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();

    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
    //  undefined – the value of the field is the initial value that also becomes the new previous value.
    // It is a dynamic error [ERR D6] if the instruction context has no initial value.

    BOOST_CHECK_THROW(decoder_operators[inst.field_operator()]->decode(ref, strm, pmap), mfast::fast_error);

    // make sure we consume one bit in presence map
    BOOST_CHECK_EQUAL(old_mask >> 1 , pmap.mask());

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with initial value
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x76\x61\x6C\x75\xE5";
    fast_istream strm(data, 6);
    presence_map pmap;
    // Optional string and byte vector fields – one bit.
    // The tail value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty.
    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 5);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we consume one bit in presence map
    BOOST_CHECK_EQUAL(old_mask >> 1 , pmap.mask());

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
   //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_svalue"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    ascii_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_svalue"));

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with NULL tail value
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\xC0\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional string and byte vector fields – one bit.
    // The tail value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty.
    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we consume one bit in presence map
    BOOST_CHECK_EQUAL(old_mask >> 1 , pmap.mask());

    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 0);

    ascii_string_cref prev(&inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with initial value while tail value not in the stream
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value,
                                 default_len);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional string and byte vector fields – one bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we consume one bit in presence map
    BOOST_CHECK_EQUAL(old_mask >> 1 , pmap.mask());

    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
   //  undefined – the value of the field is the initial value that also becomes the new previous value..
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("initial_string"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    ascii_string_mref prev(&alloc, &inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("initial_string"));

    // change the previous value to "ABCDE" so we can verified the case with defined previous value
    prev = "ABCDE";

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
   //  assigned – the value of the field is the previous value.
    BOOST_CHECK(ref.present());
    BOOST_CHECK_PREDICATE(str_equal, (ref.c_str())("ABCDE"));
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.present());
    BOOST_CHECK_PREDICATE(str_equal, (prev.c_str()) ("ABCDE"));

    prev.as_absent();

    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);
    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way:
    // empty – the value of the field is empty. If the field is optional the value is considered absent.
    BOOST_CHECK(ref.absent());
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());


    inst.destruct_value(storage, &alloc);

  }
  { // testing optional field without initial value while tail value not in the stream
    const char* default_value = "initial_string";
    const uint32_t default_len = strlen(default_value);
    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0);

    inst.construct_value(storage, &alloc);

    ascii_string_mref ref(&alloc, &storage, &inst);

    char data[] = "\x80\x80";
    fast_istream strm(data, 2);
    presence_map pmap;
    // Optional string and byte vector fields – one bit.

    strm.decode(pmap);
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);
    uint64_t old_mask = pmap.mask();


    decoder_operators[inst.field_operator()]->decode(ref, strm, pmap);

    // make sure we consume one bit in presence map
    BOOST_CHECK_EQUAL(old_mask >> 1 , pmap.mask());

    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
    //  undefined – the value of the field is the initial value that also becomes the new previous value.
    // If the field has optional presence and no initial value, the field is considered absent and the state of the previous value is changed to empty.
    BOOST_CHECK(ref.absent());
    BOOST_CHECK_EQUAL(strm.in_avail(), 1);

    ascii_string_mref prev(&alloc, &inst.prev_value(), &inst);
    // check the previous value is properly set
    BOOST_CHECK(inst.prev_value().is_defined());
    BOOST_CHECK(prev.absent());
  }
}
BOOST_AUTO_TEST_SUITE_END()
