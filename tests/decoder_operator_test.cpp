// Copyright (c) 2013, 2014, Huang-Ming Huang,  Object Computing, Inc.
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
#include "catch.hpp"

#include <mfast/int_ref.h>
#include <mfast/coder/common/codec_helper.h>
#include <mfast/coder/decoder/fast_istream.h>
#include <mfast/coder/decoder/decoder_field_operator.h>
#include <cstring>
#include <stdexcept>

#include "debug_allocator.h"
#include "byte_stream.h"
#include "mfast/output.h"

#include <mfast/coder/decoder_v2/fast_decoder_core.h>

using namespace mfast;


enum pmap_status_enum_t { NO_PMAP_BIT, HAS_PMAP_BIT };

enum prev_value_status_enum_t { CHANGE_PREVIOUS_VALUE, PRESERVE_PREVIOUS_VALUE };

template <typename MREF>
bool
decode_mref(const byte_stream&       input_stream,
            pmap_status_enum_t       pmap_status,
            const MREF&              expected,
            prev_value_status_enum_t prev_status)
{
  fast_istreambuf sb(input_stream.data(), input_stream.size());
  fast_istream strm(&sb);
  decoder_presence_map pmap;

  strm.decode(pmap);
  std::size_t old_mask = pmap.mask();

  mfast::allocator* alloc = expected.allocator();
  value_storage storage;

  typename MREF::instruction_cptr instruction = expected.instruction();
  instruction->construct_value(storage, alloc);

  value_storage old_prev_storage;
  if (instruction->prev_value().is_defined())
    instruction->copy_construct_value(instruction->prev_value(),
                                      old_prev_storage,
                                      alloc);

  typename MREF::cref_type old_prev( &old_prev_storage, instruction);

  MREF ref(alloc, &storage, instruction);

  decoder_operators[instruction->field_operator()]->decode(ref, strm, pmap);

  bool pmap_check_successful = false;

  if (pmap_status == NO_PMAP_BIT) {
    pmap_check_successful = (old_mask == pmap.mask());
  }
  else {  // pmap_status == HAS_PMAP_BIT
    pmap_check_successful = ((old_mask >> 1) == pmap.mask());
  }

  bool res( true );

  typename MREF::cref_type prev( &instruction->prev_value(), instruction);

  if (!pmap_check_successful) {
    res = false;
    INFO(  "pmap consume bit error." );
  }
  else if (expected  != ref) {
    res = false;
    INFO( "decoded value " << ref << " does not match expected" );
  }
  else if (prev_status == PRESERVE_PREVIOUS_VALUE) {
    if (!old_prev_storage.is_defined()) {
      if (instruction->prev_value().is_defined()) {
        res = false;
        INFO(  "previous value changed after decoding" );
      }
    }
    else if (!instruction->prev_value().is_defined()) {
      res = false;
      INFO(  "previous value not defined after decoding" );
    }
    else if (old_prev != prev) {
      res = false;
      INFO(  "previous value changed after decoding" );
    }
  }
  else if (prev != ref) {
    res = false;
    INFO(  "previous value is not properly set after decoding" );
  }


  instruction->destruct_value(old_prev_storage, alloc);
  instruction->destruct_value(storage, alloc);

  return res;
}

template <typename EXT_MREF>
bool
decode_ext_mref(const byte_stream&       input_stream,
                pmap_status_enum_t       pmap_status,
                const EXT_MREF&          expected_ext_ref,
                prev_value_status_enum_t prev_status)
{
  mfast::malloc_allocator allocator;
  mfast::allocator* alloc = &allocator;

  mfast::coder::fast_decoder_core<0> core(alloc);

  fast_istreambuf sb(input_stream.data(), input_stream.size());
  core.strm_.reset(&sb);

  decoder_presence_map pmap;

  typedef typename  EXT_MREF::cref_type CREF;
  CREF expected = expected_ext_ref.get();

  core.strm_.decode(pmap);
  std::size_t old_mask = pmap.mask();


  value_storage storage;

  typename CREF::instruction_cptr instruction = expected.instruction();
  instruction->construct_value(storage, alloc);

  value_storage old_prev_storage;
  if (instruction->prev_value().is_defined())
    instruction->copy_construct_value(instruction->prev_value(),
                                      old_prev_storage,
                                      alloc);

  CREF old_prev( &old_prev_storage, instruction);

  typename EXT_MREF::mref_type ref(alloc, &storage, instruction);

  core.current_ = &pmap;
  core.visit(EXT_MREF(ref));

  bool pmap_check_successful = false;

  if (pmap_status == NO_PMAP_BIT) {
    pmap_check_successful = (old_mask == pmap.mask());
  }
  else {  // pmap_status == HAS_PMAP_BIT
    pmap_check_successful = ((old_mask >> 1) == pmap.mask());
  }

  bool res( true );

  CREF prev( &instruction->prev_value(), instruction);

  if (!pmap_check_successful) {
    res = false;
    INFO(  "pmap consume bit error." );
  }
  else if (expected  != ref) {
    res = false;
    INFO( "decoded value " << ref << " does not match expected" );
  }
  else if (prev_status == PRESERVE_PREVIOUS_VALUE) {
    if (!old_prev_storage.is_defined()) {
      if (instruction->prev_value().is_defined()) {
        res = false;
        INFO(  "previous value changed after decoding" );
      }
    }
    else if (!instruction->prev_value().is_defined()) {
      res = false;
      INFO(  "previous value not defined after decoding" );
    }
    else if (old_prev != prev) {
      res = false;
      INFO(  "previous value changed after decoding" );
    }
  }
  else if (prev != ref) {
    res = false;
    INFO(  "previous value is not properly set after decoding" );
  }


  instruction->destruct_value(old_prev_storage, alloc);
  instruction->destruct_value(storage, alloc);
  return res;
}


typedef properties_type<3> optional_with_initial_value_tag;
typedef properties_type<1> optional_without_initial_value_tag;
typedef properties_type<0> mandatory_without_initial_value_tag;
typedef properties_type<2> mandatory_with_initial_value_tag;

TEST_CASE("test the decoding of fast operator none","[operator_none_decode_test]")
{
  malloc_allocator allocator;
  value_storage storage;

  {
    uint64_field_instruction inst(operator_none,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);

    // If a field is optional and has no field operator, it is encoded with a
    // nullable representation and the NULL is used to represent absence of a
    // value. It will not occupy any bits in the presence map.

    uint64_mref result(&allocator, &storage, &inst);

    result.omit();
    REQUIRE( decode_mref("\xC0\x80", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again

    REQUIRE( decode_ext_mref("\xC0\x80",
                                 NO_PMAP_BIT,
                                 ext_mref<uint64_mref, none_operator_tag, optional_with_initial_value_tag>(result),
                                 CHANGE_PREVIOUS_VALUE) );

  }
  {
    uint64_field_instruction inst(operator_none,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);

    // If a field is optional and has no field operator, it is encoded with a
    // nullable representation and the NULL is used to represent absence of a
    // value. It will not occupy any bits in the presence map.

    uint64_mref result(&allocator, &storage, &inst);
    result.as(0);
    REQUIRE( decode_mref("\xC0\x80", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE( decode_ext_mref("\xC0\x80",
                                 NO_PMAP_BIT,
                                 ext_mref<uint64_mref, none_operator_tag, mandatory_with_initial_value_tag >(result),
                                 CHANGE_PREVIOUS_VALUE) );
  }
}

TEST_CASE("test the decoding of fast operator constant","[operator_constant_decode_test]")
{

  malloc_allocator allocator;
  value_storage storage;

  {
    // An optional field with the constant operator will occupy a single bit. If the bit is set, the value
    // is the initial value in the instruction context. If the bit is not set, the value is considered absent.

    uint64_field_instruction inst(operator_constant,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.as(UINT64_MAX);

    // testing when the presence bit is set
    REQUIRE( decode_mref("\xC0\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE( decode_ext_mref("\xC0\x80",
                                 HAS_PMAP_BIT,
                                 ext_mref<uint64_mref, constant_operator_tag, optional_with_initial_value_tag>(result),
                                 CHANGE_PREVIOUS_VALUE) );

    // testing when the presence bit is not set

    result.omit();
    REQUIRE( decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    REQUIRE( decode_ext_mref("\x80\x80",
                                 HAS_PMAP_BIT,
                                 ext_mref<uint64_mref, constant_operator_tag, optional_with_initial_value_tag>(result),
                                 CHANGE_PREVIOUS_VALUE) );
  }
  {
    // A field will not occupy any bit in the presence map if it is mandatory and has the constant operator.

    uint64_field_instruction inst(operator_constant,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    REQUIRE( !result.optional());

    result.as(UINT64_MAX);
    REQUIRE( decode_mref("\xC0\x80", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE( decode_ext_mref("\xC0\x80",
                                 NO_PMAP_BIT,
                                 ext_mref<uint64_mref, constant_operator_tag, mandatory_with_initial_value_tag >(result),
                                 CHANGE_PREVIOUS_VALUE) );
  }
}

TEST_CASE("test the decoding of fast operator default","[operator_default_decode_test]")
{

  malloc_allocator allocator;
  value_storage storage;

  {
    uint64_field_instruction inst(operator_default,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.
    result.as(0);
    REQUIRE(decode_mref("\xC0\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    REQUIRE(decode_ext_mref("\xC0\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, default_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);

    result.as(UINT64_MAX);
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, default_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);

    result.omit();
    // Optional integer, decimal, string and byte vector fields – one bit.
    // If set, the value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous
    // value  is left unchanged.
    REQUIRE(decode_mref("\xC0\x80", HAS_PMAP_BIT, result, PRESERVE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, default_operator_tag, optional_with_initial_value_tag>(result), PRESERVE_PREVIOUS_VALUE) );
  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);

    // The default operator specifies that the value of a field is either present in the stream or it will be the initial value.

    result.as(UINT64_MAX);
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, default_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );
  }

  {
    uint64_field_instruction inst(operator_default,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr, int_value_storage<uint64_t>());
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);

    // If the field has optional presence and no initial value, the field is considered absent when there is no value in the stream.

    result.omit();
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, default_operator_tag, optional_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

  }
}

TEST_CASE("test the decoding of fast operator copy","[operator_copy_decode_test]")
{
  malloc_allocator allocator;
  value_storage storage;

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.as(0);

    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.
    REQUIRE(decode_mref("\xC0\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);

    uint64_mref result(&allocator, &storage, &inst);
    result.as(UINT64_MAX);

    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

    uint64_mref prev(&allocator, &inst.prev_value(), &inst);
    /// set previous value to a different value
    prev.as(5);
    result.as(5);

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value.
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    prev.as(5);
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

    /// set previous value to empty
    inst.prev_value().present(false);    // // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // // * empty – the value of the field is empty. If the field is optional the value is considered absent. It is a dynamic error [ERR D6] if the field is mandatory.

    REQUIRE_THROWS_AS(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE),                                                                                  mfast::fast_error );
    inst.prev_value().present(false);
    REQUIRE_THROWS_AS(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE), mfast::fast_error );
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.omit();
    // Optional integer, decimal, string and byte vector fields – one bit.
    // If set, the value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty
    REQUIRE(decode_mref("\xC0\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.as(UINT64_MAX);
    // Optional integer, decimal, string and byte vector fields – one bit.
    // If set, the value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );


    uint64_mref prev(&allocator, &inst.prev_value(), &inst);
    /// set previous value to a different value
    prev.as(5);

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value.
    result.as(5);
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    prev.as(5);
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    /// set previous value to empty
    inst.prev_value().present(false);
    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * empty – the value of the field is empty. If the field is optional the value is considered absent.
    result.omit();
  }

  { // testing no initial value
    uint64_field_instruction inst(operator_copy,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr, int_value_storage<uint64_t>());
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.omit();

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – If the field has optional presence and no initial value, the field is considered absent and the state of the previous value is changed to empty.
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, copy_operator_tag, optional_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
  }
}

TEST_CASE("test the decoding of fast operator increment","[operator_increment_decode_test]")
{
  malloc_allocator allocator;
  value_storage storage;

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.as(0);
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.

    REQUIRE(decode_mref("\xC0\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    REQUIRE(decode_ext_mref("\xC0\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

  }

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.as(UINT64_MAX);
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );

    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

    uint64_mref prev(&allocator, &inst.prev_value(), &inst);
    /// set previous value to a different value
    prev.as(5);

    result.as(6);
    // Mandatory integer, decimal, string and byte vector fields – one bit. If set, the value appears in the stream.

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value incremented by one. The incremented value also becomes the new previous value.
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    prev.as(5);
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
    /// set previous value to empty
    inst.prev_value().present(false);
    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * empty – the value of the field is empty. If the field is optional the value is considered absent. It is a dynamic error [ERR D6] if the field is mandatory.
    REQUIRE_THROWS_AS(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE),                                                                                       mfast::fast_error);
    REQUIRE_THROWS_AS(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE), mfast::fast_error);
  }

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.omit();
    // Optional integer, decimal, string and byte vector fields – one bit.
    // If set, the value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty

    REQUIRE(decode_mref("\xC0\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    REQUIRE(decode_ext_mref("\xC0\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );
  }

  {
    uint64_field_instruction inst(operator_increment,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.as(UINT64_MAX);
    // Optional integer, decimal, string and byte vector fields – one bit.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – the value of the field is the initial value that also becomes the new previous value.
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    uint64_mref prev(&allocator, &inst.prev_value(), &inst);

    /// set previous value to a different value
    prev.as(5);
    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * assigned – the value of the field is the previous value incremented by one. The incremented value also becomes the new previous value.

    result.as(6);
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    /// set previous value to a different value
    prev.as(5);
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    /// set previous value to empty
    prev.omit();
    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * empty – the value of the field is empty. If the field is optional the value is considered absent.
    result.omit();
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );
  }

  { // testing no initial value
    uint64_field_instruction inst(operator_increment,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr, int_value_storage<uint64_t>());
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    result.omit();
    // Optional integer, decimal, string and byte vector fields – one bit.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty

    // When the value is not present in the stream there are three cases depending on the state of the previous value:
    // * undefined – If the field has optional presence and no initial value, the field is considered absent and the state of the previous value is changed to empty.

    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<uint64_mref, increment_operator_tag, optional_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
  }
}

TEST_CASE("test the decoding of fast operator delta for integers","[operator_delta_integer_decode_test]")
{
  malloc_allocator allocator;
  value_storage storage;

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(5ULL));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.


    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as(7);
    REQUIRE(decode_mref("\xC0\x82", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x82", NO_PMAP_BIT, ext_mref<uint64_mref, delta_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

  }

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr, int_value_storage<uint64_t>()); // no initial value
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.


    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.

    result.as(2);
    REQUIRE(decode_mref("\xC0\x82", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x82", NO_PMAP_BIT, ext_mref<uint64_mref, delta_operator_tag, mandatory_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

  }

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(5ULL));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    //Optional integer, decimal, string and byte vector fields – no bit.
    // The delta appears in the stream in a nullable representation. A NULL indicates that the delta is absent.


    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.

    result.as(6);
    REQUIRE(decode_mref("\xC0\x82", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x82", NO_PMAP_BIT, ext_mref<uint64_mref, delta_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

  }

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(5ULL));
    inst.construct_value(storage, &allocator);


    uint64_mref result(&allocator, &storage, &inst);
    //Optional integer, decimal, string and byte vector fields – no bit.
    // The delta appears in the stream in a nullable representation. A NULL indicates that the delta is absent.


    //  If the field has optional presence, the delta value can be NULL. In that case the value of the field is considered absent.

    result.omit();
    REQUIRE(decode_mref("\xC0\x80", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80", NO_PMAP_BIT, ext_mref<uint64_mref, delta_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );
  }

}

TEST_CASE("test the decoding of fast operator delta for decimal","[operator_delta_decimal_decode_test]")
{
  malloc_allocator allocator;
  value_storage storage;

  {
    decimal_field_instruction inst(operator_delta,
                                   presence_mandatory,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   decimal_value_storage(12,1)); //  initial

    inst.construct_value(storage, &allocator);

    decimal_mref result(&allocator, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as(15,3); // 15 = 12 (base) + 3 (delta), 3= 1 (base) + 2 (delta)
    REQUIRE(decode_mref("\xC0\x82\x83", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x82\x83", NO_PMAP_BIT, ext_mref<decimal_mref, delta_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
  }
  {
    decimal_field_instruction inst(operator_delta,
                                   presence_mandatory,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   decimal_value_storage()); // no initial value

    inst.construct_value(storage, &allocator);
    decimal_mref result(&allocator, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as(3,2); // 3 =  0 (base) + 3 (delta), 2 = 0 (base) + 2 (delta)
    REQUIRE(decode_mref("\xC0\x82\x83", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x82\x83", NO_PMAP_BIT, ext_mref<decimal_mref, delta_operator_tag, mandatory_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
  }

  {
    decimal_field_instruction inst(operator_delta,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   decimal_value_storage()); // no initial value

    inst.construct_value(storage, &allocator);
    decimal_mref result(&allocator, &storage, &inst);
    // Optional integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.omit();
    REQUIRE(decode_mref("\xC0\x80\x83", NO_PMAP_BIT, result, PRESERVE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80\x83", NO_PMAP_BIT, ext_mref<decimal_mref, delta_operator_tag, optional_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
  }
}

TEST_CASE("test the decoding of fast operator delta for ascii string","[operator_delta_ascii_decode_test]")
{
  debug_allocator alloc;
  value_storage storage;

  { // testing mandatory field with initial value
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_delta,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as("initial_value");
    REQUIRE(decode_mref("\xC0\x86\x76\x61\x6C\x75\xE5", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x86\x76\x61\x6C\x75\xE5", NO_PMAP_BIT, ext_mref<ascii_string_mref, delta_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }

  { // testing mandatory field without initial value

    ascii_field_instruction inst(operator_delta,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 nullptr, string_value_storage());

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as("ABCD");
    REQUIRE(decode_mref("\x80\x80\x41\x42\x43\xC4", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80\x41\x42\x43\xC4", NO_PMAP_BIT, ext_mref<ascii_string_mref, delta_operator_tag, mandatory_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }

  { // testing mandatory field without initial value
    // const char* default_value = "initial_string";

    ascii_field_instruction inst(operator_delta,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 nullptr, string_value_storage());

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    REQUIRE_THROWS_AS(decode_mref("\xC0\x86\x76\x61\x6C\x75\xE5",
                                  NO_PMAP_BIT,
                                  result,
                                  CHANGE_PREVIOUS_VALUE),                mfast::fast_error);
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE_THROWS_AS(decode_ext_mref("\xC0\x86\x76\x61\x6C\x75\xE5", NO_PMAP_BIT, ext_mref<ascii_string_mref, delta_operator_tag, mandatory_without_initial_value_tag >(
                                        result), CHANGE_PREVIOUS_VALUE), mfast::fast_error);

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with NULL substraction in the stream
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_delta,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Optional integer, decimal, string and byte vector fields – no bit.

    // If the field has optional presence, the delta value can be NULL.
    // In that case the value of the field is considered absent.
    // Note that the previous value is not set to empty but is left untouched if the value is absent.
    result.omit();
    REQUIRE(decode_mref("\xC0\x80", NO_PMAP_BIT, result, PRESERVE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80", NO_PMAP_BIT, ext_mref<ascii_string_mref, delta_operator_tag, optional_with_initial_value_tag>(result), PRESERVE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }
  { // testing optional field with positive substraction in the stream
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_delta,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Optional integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.

    result.as("initial_striABCD");
    REQUIRE(decode_mref("\xC0\x83\x41\x42\x43\xC4", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x83\x41\x42\x43\xC4", NO_PMAP_BIT, ext_mref<ascii_string_mref, delta_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with negative substraction in the stream
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_delta,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Optional integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.

    result.as("ABCD_string");
    REQUIRE(decode_mref("\x80\xF8\x41\x42\x43\xC4", NO_PMAP_BIT,  result,  CHANGE_PREVIOUS_VALUE ) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\xF8\x41\x42\x43\xC4", NO_PMAP_BIT, ext_mref<ascii_string_mref, delta_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }
}

TEST_CASE("test the decoding of fast operator delta for unicode string","[operator_delta_unicode_decode_test]")
{
  debug_allocator alloc;
  value_storage storage;

  { // testing mandatory field with initial value
    const char* default_value = "initial_string";
    unicode_field_instruction inst(operator_delta,
                                   presence_mandatory,
                                   1,
                                   "test_ascii","",
                                   nullptr,
                                   string_value_storage(default_value), 0, "", "");

    inst.construct_value(storage, &alloc);

    unicode_string_mref result(&alloc, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.

    result.as("initial_value");
    REQUIRE(decode_mref("\xC0\x86\x85\x76\x61\x6C\x75\x65", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x86\x85\x76\x61\x6C\x75\x65", NO_PMAP_BIT, ext_mref<unicode_string_mref, delta_operator_tag, mandatory_with_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }

  { // testing mandatory field without initial value
    //const char* default_value = "initial_string";

    unicode_field_instruction inst(operator_delta,
                                   presence_mandatory,
                                   1,
                                   "test_unicode","",
                                   nullptr,
                                   string_value_storage(), 0, "", "");

    inst.construct_value(storage, &alloc);

    unicode_string_mref result(&alloc, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.

    // result.as("initial_value");
    REQUIRE_THROWS_AS(decode_mref("\xC0\x86\x85\x76\x61\x6C\x75\x65", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE),
                      mfast::fast_error );
    REQUIRE_THROWS_AS(decode_ext_mref("\xC0\x86\x85\x76\x61\x6C\x75\x65", NO_PMAP_BIT,
                                      ext_mref<unicode_string_mref, delta_operator_tag, mandatory_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE),
                      mfast::fast_error );
    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with NULL substraction in the stream
    const char* default_value = "initial_string";
    unicode_field_instruction inst(operator_delta,
                                   presence_optional,
                                   1,
                                   "test_unicode","",
                                   nullptr,
                                   string_value_storage(default_value), 0, "", "");

    inst.construct_value(storage, &alloc);

    unicode_string_mref result(&alloc, &storage, &inst);
    // Mandatory integer, decimal, string and byte vector fields – no bit.

    // Optional integer, decimal, string and byte vector fields – no bit.
    // The delta appears in the stream in a nullable representation.
    // A NULL indicates that the delta is absent.
    // Note that the previous value is not set to empty but is left untouched if the value is absent.


    result.omit();
    REQUIRE(decode_mref("\xC0\x80", NO_PMAP_BIT, result, PRESERVE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80", NO_PMAP_BIT, ext_mref<unicode_string_mref, delta_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with positive substraction in the stream
    const char* default_value = "initial_string";
    unicode_field_instruction inst(operator_delta,
                                   presence_optional,
                                   1,
                                   "test_unicode","",
                                   nullptr,
                                   string_value_storage(default_value), 0, "", "");

    inst.construct_value(storage, &alloc);

    unicode_string_mref result(&alloc, &storage, &inst);
    // Optional integer, decimal, string and byte vector fields – no bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.

    result.as("initial_striABCD");
    REQUIRE(decode_mref("\xC0\x83\x84\x41\x42\x43\x44", NO_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x83\x84\x41\x42\x43\x44", NO_PMAP_BIT, ext_mref<unicode_string_mref, delta_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }
}

TEST_CASE("test the decoding of fast operator tail for ascii string","[operator_tail_ascii_decode_test]")
{
  debug_allocator alloc;
  value_storage storage;

  { // testing mandatory field with initial value
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_tail,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));
    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Mandatory string and byte vector fields – one bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as("initial_svalue");
    REQUIRE(decode_mref("\xC0\x76\x61\x6C\x75\xE5", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }
  { // testing mandatory field with initial value while tail value not in the stream
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_tail,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Mandatory string and byte vector fields – one bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as("initial_string");
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    memset(const_cast<value_storage*>(&inst.prev_value()), 0, sizeof(value_storage));
    ascii_string_mref prev(&alloc, &inst.prev_value(), &inst);

    // change the previous value to "ABCDE" so we can verified the case with defined previous value
    inst.prev_value().defined(true);
    prev = "ABCDE";
    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
    //  assigned – the value of the field is the previous value.
    result.as("ABCDE");
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(const_cast<value_storage&>(inst.prev_value()), &alloc);
    inst.destruct_value(storage, &alloc);

  }
  { // testing mandatory field without initial value while tail value not in the stream

    ascii_field_instruction inst(operator_tail,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 nullptr, string_value_storage());

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &inst.prev_value(), &inst);
    // Mandatory string and byte vector fields – one bit.

    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
    //  undefined – the value of the field is the initial value that also becomes the new previous value.
    // It is a dynamic error [ERR D6] if the instruction context has no initial value.

    REQUIRE_THROWS_AS(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE), mfast::fast_error );

    inst.destruct_value(storage, &alloc);
  }

  { // testing mandatory field without initial value while tail value is in the stream

    ascii_field_instruction inst(operator_tail,
                                 presence_mandatory,
                                 1,
                                 "test_ascii","",
                                 nullptr, string_value_storage());

    inst.construct_value(storage, &alloc);
    ascii_string_mref result(&alloc, &storage, &inst);

    // Mandatory string and byte vector fields – one bit.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as("value");


    REQUIRE(decode_mref("\xC0\x76\x61\x6C\x75\xE5", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x76\x61\x6C\x75\xE5", HAS_PMAP_BIT, ext_mref<ascii_string_mref, tail_operator_tag, mandatory_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);

  }

  { // testing optional field with initial value
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Optional string and byte vector fields – one bit.
    // The tail value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.as("initial_svalue");
    REQUIRE(decode_mref("\xC0\x76\x61\x6C\x75\xE5", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x76\x61\x6C\x75\xE5", HAS_PMAP_BIT, ext_mref<ascii_string_mref, tail_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with NULL tail value
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Optional string and byte vector fields – one bit.
    // The tail value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty.

    // the field is obtained by combining the delta value with a base value.
    // The base value depends on the state of the previous value in the following way:
    //  undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    result.omit();
    REQUIRE(decode_mref("\xC0\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\xC0\x80", HAS_PMAP_BIT, ext_mref<ascii_string_mref, tail_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    inst.destruct_value(storage, &alloc);
  }

  { // testing optional field with initial value while tail value not in the stream
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Optional string and byte vector fields – one bit.
    // The tail value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty.

    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
    //  undefined – the value of the field is the initial value that also becomes the new previous value..

    result.as("initial_string");
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<ascii_string_mref, tail_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );


    ascii_string_mref prev(&alloc, &inst.prev_value(), &inst);
    // change the previous value to "ABCDE" so we can verified the case with defined previous value
    prev.refers_to( "ABCDE" );
    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
    //  assigned – the value of the field is the previous value.
    result.as("ABCDE");
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    prev.refers_to( "ABCDE" );
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<ascii_string_mref, tail_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );

    prev.omit();
    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way:
    // empty – the value of the field is empty. If the field is optional the value is considered absent.
    result.omit();
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    prev.omit();
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<ascii_string_mref, tail_operator_tag, optional_with_initial_value_tag>(result), CHANGE_PREVIOUS_VALUE) );
    inst.destruct_value(storage, &alloc);

  }
  { // testing optional field without initial value while tail value not in the stream
    // const char* default_value = "initial_string";

    ascii_field_instruction inst(operator_tail,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr, string_value_storage());

    inst.construct_value(storage, &alloc);

    ascii_string_mref result(&alloc, &storage, &inst);
    // Optional string and byte vector fields – one bit.
    // The tail value appears in the stream in a nullable representation.
    // A NULL indicates that the value is absent and the state of the previous value is set to empty.

    // If the tail value is not present in the stream, the value of the field depends on the state of the previous value in the following way::
    //  undefined – the value of the field is the initial value that also becomes the new previous value.
    // If the field has optional presence and no initial value, the field is considered absent and the state of the previous value is changed to empty.

    result.omit();
    REQUIRE(decode_mref("\x80\x80", HAS_PMAP_BIT, result, CHANGE_PREVIOUS_VALUE) );
    inst.prev_value().defined(false); // reset the previous value to undefined again
    REQUIRE(decode_ext_mref("\x80\x80", HAS_PMAP_BIT, ext_mref<ascii_string_mref, tail_operator_tag, optional_without_initial_value_tag >(result), CHANGE_PREVIOUS_VALUE) );
    inst.destruct_value(storage, &alloc);
  }

}
