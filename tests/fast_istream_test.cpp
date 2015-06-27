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
#include <mfast/coder/decoder/fast_istream.h>
#include <mfast/coder/decoder/fast_istream_extractor.h>
#include <mfast/output.h>
#include "debug_allocator.h"
#include <stdexcept>
#include "byte_stream.h"

using namespace mfast;

template <typename T>
bool
decode_integer(const byte_stream& bs, bool nullable, T result)
{
  fast_istreambuf sb(bs.data(), bs.size());
  fast_istream strm(&sb);

  T value;
  bool not_null = strm.decode(value, nullable);

  if (not_null &&  value == result)
    return true;

  if (not_null) {
    INFO(  "Got \"" << value << "\" instead." );
  }
  else {
    INFO(  "Got null instead." );
  }
  return false;

}



TEST_CASE("test the fast decoding of integers","[int32_test]")
{

  REQUIRE(decode_integer( "\x39\x45\xa4", true, INT32_C(942755)));
  REQUIRE(decode_integer( "\x39\x45\xa3", false, INT32_C(942755)));
  REQUIRE(decode_integer( "\x46\x3a\xdd", true, INT32_C(-942755)));
  REQUIRE(decode_integer( "\x7c\x1b\x1b\x9d", false,INT32_C(-7942755)));
  REQUIRE(decode_integer( "\x00\x40\x81", false, INT32_C(8193)));
  REQUIRE(decode_integer( "\x7F\x3f\xff", false, INT32_C(-8193)));


  REQUIRE(decode_integer( "\x81",  true, UINT32_C(0)));
  REQUIRE(decode_integer( "\x82",  true, UINT32_C(1)));
  REQUIRE(decode_integer( "\x39\x45\xa4",  true, UINT32_C(942755)));
  REQUIRE(decode_integer( "\x80",  false, UINT32_C(0)));
  REQUIRE(decode_integer( "\x81",  false, UINT32_C(1)));
  REQUIRE(decode_integer( "\x39\x45\xa3",  false, UINT32_C(942755)));
  REQUIRE(decode_integer("\x10\x00\x00\x00\x80",  true, (std::numeric_limits<uint32_t>::max)()));

  REQUIRE(decode_integer( "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x80",  true, (std::numeric_limits<int64_t>::max)()));
  REQUIRE(decode_integer( "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80",  true, (std::numeric_limits<uint64_t>::max)()));

  { // check decoding null
    char data[] = "\x80";
    fast_istreambuf sb(data, 3);
    fast_istream strm(&sb);

    uint32_t value;
    REQUIRE(!strm.decode(value,true));
  }
}

bool
decode_string(const byte_stream& bs, bool nullable, const char* result, std::size_t result_len)
{
  ascii_field_instruction* instruction = nullptr;

  fast_istreambuf sb(bs.data(), bs.size());
  fast_istream strm(&sb);

  const char* str;
  uint32_t len;

  bool not_null = strm.decode(str, len, instruction, nullable);

  if ((str == nullptr && not_null == false) || (str && len == result_len && memcmp(str, result, len) == 0) )
    return true;

  bool res( false );
  if (not_null) {
    INFO(  "Got \"" << byte_stream(str, len) << "\" instead." );
  }
  else {
    INFO(  "Got null instead." );
  }
  return false;
}

TEST_CASE("test the fast decoding of ascii string","[ascii_string_test]")
{
  REQUIRE(decode_string( "\x80", false, "",  0));
  REQUIRE(decode_string( "\x00\x80", false, "\x0",  1));

  REQUIRE(decode_string( "\x80", true, nullptr,  0));
  REQUIRE(decode_string( "\x00\x80", true, "",  0));
  REQUIRE(decode_string( "\x00\x00\x80", true, "\x0",  1));

  REQUIRE(decode_string( "\x40\x40\xC0", true, "\x40\x40\xC0",  3));
  REQUIRE(decode_string( "\x40\x40\xC0", false, "\x40\x40\xC0",  3));

  REQUIRE_THROWS_AS(decode_string("\x00\xC0", false, nullptr, 0),    mfast::fast_error );
  REQUIRE_THROWS_AS(decode_string("\x00\xC0", true, nullptr, 0),     mfast::fast_error );
  REQUIRE_THROWS_AS(decode_string("\x00\x00\xC0", true, nullptr, 0), mfast::fast_error );
}

bool
decode_byte_vector(const byte_stream& bs, bool nullable, const char* result, std::size_t result_len)
{

  fast_istreambuf sb(bs.data(), bs.size());
  fast_istream strm(&sb);

  const unsigned char* str=nullptr;
  uint32_t len;

  bool not_null = strm.decode(str, len, nullptr, nullable);

  if ((result == nullptr && not_null == false) || (str && result && len == result_len && memcmp(str, result, len) == 0) )
    return true;

  if (not_null) {
    INFO(  "Got \"" << byte_stream(reinterpret_cast<const char*>(str), len) << "\" instead." );
  }
  else {
    INFO(  "Got null instead." );
  }
  return false;
}

TEST_CASE("test the fast decoding of byteVector","[byte_vector_test]")
{
  REQUIRE(decode_byte_vector( "\x80", true, nullptr,  0)); // null
  REQUIRE(decode_byte_vector( "\x80", false, "",  0)); // empty byte vector
  REQUIRE(decode_byte_vector( "\x81", true, "",  0)); // empty byte vector

  REQUIRE(decode_byte_vector( "\x81\xC0", false, "\xC0",  1));
  REQUIRE(decode_byte_vector( "\x82\xC0", true, "\xC0",  1));
}

template <typename T>
bool
extract_from_stream(const byte_stream& bs, const T& result)
{
  fast_istreambuf sb(bs.data(), bs.size());
  fast_istream strm(&sb);

  debug_allocator alloc;
  value_storage storage;

  result.instruction()->construct_value(storage, &alloc);

  T mref(&alloc, &storage, result.instruction());
  strm >> mref;


  if (mref == result) {
    result.instruction()->destruct_value(storage, &alloc);
    return true;
  }

  if (mref.present()) {
    INFO(  "Extract failure!\nGot \"" << mref << "\" instead." );
  }
  else {
    INFO(  "Extract failure!\nGot absent value instead." );
  }
  result.instruction()->destruct_value(storage, &alloc);
  return false;
}

TEST_CASE("test the extractors of fast_ostream","[extractor_test]")
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

    REQUIRE(extract_from_stream("\x41\x41\xC1", mref));
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
    REQUIRE(extract_from_stream("\x80", mref));

    mref.as(1, 4);
    REQUIRE(extract_from_stream("\x85\x81", mref));
  }

}

// result_bits is the pmap with stop bits removed and the most signicant bit of result_bits[0] is
// the first bit of the pmap
bool
decode_pmap(const byte_stream& bs, const char* result_bits, std::size_t maxbits)
{
  fast_istreambuf sb(bs.data(), bs.size());
  fast_istream strm(&sb);


  decoder_presence_map pmap;
  strm.decode(pmap);

  char bits[16]="";

  char* pos = bits;

  for (std::size_t i = 0; i < maxbits; ++i) {
    pos[0] <<=1;
    pos[0] |= static_cast<int>(pmap.is_next_bit_set());

    if ( (i % 8) == 7) {
      pos += 1;
    }
  }

  pos[0] <<= ( 8 - (maxbits%8) );

  std::size_t nbytes = (maxbits + 7)/8; // i.e. ceiling(maxbits/8)
  if (memcmp(bits, result_bits, nbytes) == 0)
    return true;

  INFO(  "Got \"" << byte_stream(bits, nbytes) << "\" instead." );
  return false;
}

TEST_CASE("test the fast decoding of presence map","[decoder_presence_map_test]")
{
  REQUIRE( decode_pmap( "\xC0", "\x80", 7) );
  REQUIRE( decode_pmap( "\x40\x81", "\x80\x04",  14 ) );
  REQUIRE( decode_pmap( "\x40\x40\x40\x40\x40\x40\x40\x40\xC0", "\x81\x02\x04\x08\x10\x20\x40\x80",  63 ) );
}

