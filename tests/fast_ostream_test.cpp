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

#include <mfast/coder/encoder/fast_ostream.h>
#include <mfast/coder/encoder/fast_ostream_inserter.h>
#include <mfast/coder/encoder/encoder_presence_map.h>
#include <mfast/output.h>
#include "debug_allocator.h"
#include <stdexcept>
#include "byte_stream.h"

using namespace mfast;

template <typename T>
bool
encode_integer(T value, bool nullable, const byte_stream& result)
{
  char buffer[16];
  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);

  strm.encode(value, false, nullable);

  if (byte_stream(sb) == result)
    return true;

  INFO("Got \"" << byte_stream(sb) << "\" instead.");
  return false;

}



TEST_CASE("test fast encoding for integers","[int_test]")
{

  REQUIRE(encode_integer<int16_t>(2, false, "\x82") );
  REQUIRE(encode_integer<int16_t>(2, true, "\x83") );
  REQUIRE(encode_integer<int16_t>(-2, false, "\xFE") );
  REQUIRE(encode_integer<int16_t>(-2, true, "\xFE") );
  REQUIRE(encode_integer<int16_t>(-1, true, "\xFF") );

  REQUIRE(encode_integer(INT32_C(942755), true, "\x39\x45\xa4" ));
  REQUIRE(encode_integer(INT32_C(942755), false, "\x39\x45\xa3"));
  REQUIRE(encode_integer(INT32_C(-942755), true, "\x46\x3a\xdd"));
  REQUIRE(encode_integer(INT32_C(-7942755), false, "\x7c\x1b\x1b\x9d"));
  REQUIRE(encode_integer(INT32_C(8193), false, "\x00\x40\x81"));
  REQUIRE(encode_integer(INT32_C(-8193), false, "\x7F\x3f\xff"));

  REQUIRE(encode_integer(UINT32_C(0), true, "\x81"));
  REQUIRE(encode_integer(UINT32_C(1), true, "\x82"));
  REQUIRE(encode_integer(UINT32_C(942755), true, "\x39\x45\xa4"));
  REQUIRE(encode_integer(UINT32_C(0), false, "\x80"));
  REQUIRE(encode_integer(UINT32_C(1), false, "\x81"));
  REQUIRE(encode_integer(UINT32_C(942755), false, "\x39\x45\xa3"));
  REQUIRE(encode_integer((std::numeric_limits<uint32_t>::max)(), true,"\x10\x00\x00\x00\x80"));

  REQUIRE(encode_integer((std::numeric_limits<int64_t>::max)(), true, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x80"));
  REQUIRE(encode_integer((std::numeric_limits<uint64_t>::max)(), true, "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80"));
}


bool
encode_string(const char* str,std::size_t len, bool nullable, const byte_stream& result)
{
  char buffer[16];
  ascii_field_instruction* instruction = nullptr;

  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);


  strm.encode(str, static_cast<uint32_t>(len), instruction, nullable);

  if (byte_stream(sb) == result)
    return true;

  INFO( "Got \"" << byte_stream(sb) << "\" instead." );
  return false;
}

TEST_CASE("test fast encoding for ascii strings","[ascii_string_test]")
{
  REQUIRE(encode_string("", 0, false, "\x80"));
  REQUIRE(encode_string("\x0", 1, false, "\x00\x80"));

  REQUIRE(encode_string(nullptr, 0, true, "\x80"));
  REQUIRE(encode_string("", 0, true, "\x00\x80"));
  REQUIRE(encode_string("\x0", 1, true, "\x00\x00\x80"));

  REQUIRE(encode_string("\x40\x40\xC0", 3, true, "\x40\x40\xC0"));
  REQUIRE(encode_string("\x40\x40\xC0", 3, false, "\x40\x40\xC0"));

}

bool
encode_byte_vector(const char* bv,std::size_t len, bool nullable, const byte_stream& result)
{
  char buffer[16];

  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);


  strm.encode(reinterpret_cast<const unsigned char*>(bv), static_cast<uint32_t>(len), nullptr, nullable);

  if (byte_stream(sb) == result)
    return true;

  INFO("Got \"" << byte_stream(sb) << "\" instead.");
  return false;
}

TEST_CASE("test fast encoding for byteVector","[byte_vector_test]")
{
  REQUIRE(encode_byte_vector(nullptr, 0, true, "\x80")); // null
  REQUIRE(encode_byte_vector("", 0, false, "\x80")); // empty byte vector
  REQUIRE(encode_byte_vector("", 0, true, "\x81")); // empty byte vector

  REQUIRE(encode_byte_vector("\xC0", 1, false, "\x81\xC0"));
  REQUIRE(encode_byte_vector("\xC0", 1, true, "\x82\xC0"));
}

template <typename T>
bool
insert_to_stream(const T& value, const byte_stream& result)
{
  char buffer[16];

  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);

  strm << value;

  if (byte_stream(sb) == result)
    return true;

  INFO( "Insert " << value << " failure!\nGot \"" << byte_stream(sb) << "\" instead." );
  return false;
}

TEST_CASE("test fast_ostream inserters","[inserter_test]")
{
  debug_allocator alloc;
  value_storage storage;
  {
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_copy,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);
    ascii_string_mref mref(&alloc, &storage, &inst);
    mref.refers_to("AAA");

    REQUIRE(insert_to_stream(mref, "\x41\x41\xC1"));
  }

  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   decimal_value_storage(INT64_MAX,64));

    inst.construct_value(storage, &alloc);
    decimal_mref mref(&alloc, &storage, &inst);


    mref.omit();
    REQUIRE(insert_to_stream(mref, "\x80"));

    mref.as(1, 4);
    REQUIRE(insert_to_stream(mref, "\x85\x81"));
  }

}



bool
encode_pmap(const char* bits, std::size_t maxbits, const byte_stream& result)
{
  char buffer[16];

  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);

  encoder_presence_map pmap;
  pmap.init(&strm, maxbits);

  unsigned byte=0;

  for (std::size_t i = 0; i < maxbits; ++i ) {

    if (i % 8 == 0) {
      byte = bits[0];
      bits += 1;
    }

    pmap.set_next_bit( (byte & 0x80) != 0 );
    byte <<= 1;
  }

  pmap.commit();
  if (byte_stream(sb) == result)
    return true;
  INFO ("Got \"" << byte_stream(sb) << "\" instead.");
  return false;
}

TEST_CASE("test the fast encoding of presence map","[encoder_presence_map_test]")
{
  REQUIRE( encode_pmap("\x80", 7, "\xC0" ) );
  REQUIRE( encode_pmap("\x80\x04", 14, "\x40\x81" ) );
  REQUIRE( encode_pmap("\x81\x02\x04\x08\x10\x20\x40\x80", 63, "\x40\x40\x40\x40\x40\x40\x40\x40\xC0" ) );
}

TEST_CASE("","[non_overlong_encoder_presence_map_test]")
{
  char buffer[32];

  debug_allocator alloc;

  {
    fast_ostreambuf sb(buffer);
    fast_ostream strm(&alloc);
    strm.rdbuf(&sb);

    strm.allow_overlong_pmap(false);

    encoder_presence_map pmap;
    pmap.init(&strm, 70);

    strm.encode("\x40\x41\x42\x43",
                4,
                static_cast<const ascii_field_instruction*>(nullptr),
                false);

    for (std::size_t i = 0; i < 70; ++i) {
      pmap.set_next_bit(false);
    }

    pmap.commit();

    REQUIRE (byte_stream(sb) == byte_stream("\x80\x40\x41\x42\xC3"));
  }

  {
    fast_ostreambuf sb(buffer);
    fast_ostream strm(&alloc);
    strm.rdbuf(&sb);

    strm.allow_overlong_pmap(false);

    strm.encode(0, false, false);
    encoder_presence_map pmap;
    pmap.init(&strm, 70);

    strm.encode("\x40\x41\x42\x43",
                4,
                static_cast<const ascii_field_instruction*>(nullptr),
                false);

    for (std::size_t i = 0; i < 70; ++i) {
      pmap.set_next_bit(false);
    }

    pmap.commit();

    REQUIRE (byte_stream(sb) == byte_stream("\x80\x80\x40\x41\x42\xC3"));
  }

  {
    fast_ostreambuf sb(buffer);
    fast_ostream strm(&alloc);
    strm.rdbuf(&sb);

    strm.allow_overlong_pmap(false);

    strm.encode(0, false, false);
    encoder_presence_map pmap;
    pmap.init(&strm, 71);

    strm.encode("\x40\x41\x42\x43",
                4,
                static_cast<const ascii_field_instruction*>(nullptr),
                false);

    pmap.set_next_bit(true);
    for (std::size_t i = 0; i < 70; ++i) {
      pmap.set_next_bit(false);
    }

    pmap.commit();

    REQUIRE (byte_stream(sb) == byte_stream("\x80\xC0\x40\x41\x42\xC3"));
  }
}



