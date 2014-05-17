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
#include <mfast/coder/encoder/fast_ostream.h>
#include <mfast/coder/encoder/fast_ostream_inserter.h>
#include <mfast/coder/encoder/encoder_presence_map.h>
#include <mfast/output.h>
#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

#include "debug_allocator.h"
#include <stdexcept>
#include "byte_stream.h"

using namespace mfast;

template <typename T>
boost::test_tools::predicate_result
encode_integer(T value, bool nullable, const byte_stream& result)
{
  char buffer[16];
  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);

  strm.encode(value, nullable);

  if (byte_stream(sb) == result)
    return true;

  boost::test_tools::predicate_result res( false );
  res.message() << "Got \"" << byte_stream(sb) << "\" instead.";
  return res;

}

BOOST_AUTO_TEST_SUITE( test_fast_ostream )


BOOST_AUTO_TEST_CASE(int_test)
{

  BOOST_CHECK(encode_integer<int16_t>(2, false, "\x82") );
  BOOST_CHECK(encode_integer<int16_t>(2, true, "\x83") );
  BOOST_CHECK(encode_integer<int16_t>(-2, false, "\xFE") );
  BOOST_CHECK(encode_integer<int16_t>(-2, true, "\xFE") );

  BOOST_CHECK(encode_integer(INT32_C(942755), true, "\x39\x45\xa4" ));
  BOOST_CHECK(encode_integer(INT32_C(942755), false, "\x39\x45\xa3"));
  BOOST_CHECK(encode_integer(INT32_C(-942755), true, "\x46\x3a\xdd"));
  BOOST_CHECK(encode_integer(INT32_C(-7942755), false, "\x7c\x1b\x1b\x9d"));
  BOOST_CHECK(encode_integer(INT32_C(8193), false, "\x00\x40\x81"));
  BOOST_CHECK(encode_integer(INT32_C(-8193), false, "\x7F\x3f\xff"));

  BOOST_CHECK(encode_integer(UINT32_C(0), true, "\x81"));
  BOOST_CHECK(encode_integer(UINT32_C(1), true, "\x82"));
  BOOST_CHECK(encode_integer(UINT32_C(942755), true, "\x39\x45\xa4"));
  BOOST_CHECK(encode_integer(UINT32_C(0), false, "\x80"));
  BOOST_CHECK(encode_integer(UINT32_C(1), false, "\x81"));
  BOOST_CHECK(encode_integer(UINT32_C(942755), false, "\x39\x45\xa3"));
  BOOST_CHECK(encode_integer((std::numeric_limits<uint32_t>::max)(), true,"\x10\x00\x00\x00\x80"));

  BOOST_CHECK(encode_integer((std::numeric_limits<int64_t>::max)(), true, "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x80"));
  BOOST_CHECK(encode_integer((std::numeric_limits<uint64_t>::max)(), true, "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80"));
}


boost::test_tools::predicate_result
encode_string(const char* str,std::size_t len, bool nullable, const byte_stream& result)
{
  char buffer[16];
  ascii_field_instruction* instruction = 0;

  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);


  strm.encode(str, static_cast<uint32_t>(len), nullable, instruction);

  if (byte_stream(sb) == result)
    return true;

  boost::test_tools::predicate_result res( false );
  res.message() << "Got \"" << byte_stream(sb) << "\" instead.";
  return res;
}

BOOST_AUTO_TEST_CASE(ascii_string_test)
{
  BOOST_CHECK(encode_string("", 0, false, "\x80"));
  BOOST_CHECK(encode_string("\x0", 1, false, "\x00\x80"));

  BOOST_CHECK(encode_string(0, 0, true, "\x80"));
  BOOST_CHECK(encode_string("", 0, true, "\x00\x80"));
  BOOST_CHECK(encode_string("\x0", 1, true, "\x00\x00\x80"));

  BOOST_CHECK(encode_string("\x40\x40\xC0", 3, true, "\x40\x40\xC0"));
  BOOST_CHECK(encode_string("\x40\x40\xC0", 3, false, "\x40\x40\xC0"));

}

boost::test_tools::predicate_result
encode_byte_vector(const char* bv,std::size_t len, bool nullable, const byte_stream& result)
{
  char buffer[16];

  debug_allocator alloc;
  fast_ostreambuf sb(buffer);
  fast_ostream strm(&alloc);
  strm.rdbuf(&sb);


  strm.encode(reinterpret_cast<const unsigned char*>(bv), static_cast<uint32_t>(len), nullable, 0);

  if (byte_stream(sb) == result)
    return true;

  boost::test_tools::predicate_result res( false );
  res.message() << "Got \"" << byte_stream(sb) << "\" instead.";
  return res;
}

BOOST_AUTO_TEST_CASE(byte_vector_test)
{
  BOOST_CHECK(encode_byte_vector(0, 0, true, "\x80")); // null
  BOOST_CHECK(encode_byte_vector("", 0, false, "\x80")); // empty byte vector
  BOOST_CHECK(encode_byte_vector("", 0, true, "\x81")); // empty byte vector

  BOOST_CHECK(encode_byte_vector("\xC0", 1, false, "\x81\xC0"));
  BOOST_CHECK(encode_byte_vector("\xC0", 1, true, "\x82\xC0"));
}

template <typename T>
boost::test_tools::predicate_result
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

  boost::test_tools::predicate_result res( false );
  res.message() << "Insert " << value << " failure!\nGot \"" << byte_stream(sb) << "\" instead.";
  return res;
}

BOOST_AUTO_TEST_CASE(inserter_test)
{
  debug_allocator alloc;
  value_storage storage;
  {
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_copy,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 string_value_storage(default_value));

    inst.construct_value(storage, &alloc);
    ascii_string_mref mref(&alloc, &storage, &inst);
    mref.refers_to("AAA");

    BOOST_CHECK(insert_to_stream(mref, "\x41\x41\xC1"));
  }

  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0,
                                   decimal_value_storage(INT64_MAX,64));

    inst.construct_value(storage, &alloc);
    decimal_mref mref(&alloc, &storage, &inst);


    mref.omit();
    BOOST_CHECK(insert_to_stream(mref, "\x80"));

    mref.as(1, 4);
    BOOST_CHECK(insert_to_stream(mref, "\x85\x81"));
  }

}



boost::test_tools::predicate_result
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
  boost::test_tools::predicate_result res( false );
  res.message() << "Got \"" << byte_stream(sb) << "\" instead.";
  return res;
}

BOOST_AUTO_TEST_CASE(encoder_presence_map_test)
{
  BOOST_CHECK( encode_pmap("\x80", 7, "\xC0" ) );
  BOOST_CHECK( encode_pmap("\x80\x04", 14, "\x40\x81" ) );
  BOOST_CHECK( encode_pmap("\x81\x02\x04\x08\x10\x20\x40\x80", 63, "\x40\x40\x40\x40\x40\x40\x40\x40\xC0" ) );
}

BOOST_AUTO_TEST_CASE(non_overlong_encoder_presence_map_test)
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
                false,
                static_cast<const ascii_field_instruction*>(0));

    for (std::size_t i = 0; i < 70; ++i) {
      pmap.set_next_bit(false);
    }

    pmap.commit();

    BOOST_CHECK (byte_stream(sb) == byte_stream("\x80\x40\x41\x42\xC3"));
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
                false,
                static_cast<const ascii_field_instruction*>(0));

    for (std::size_t i = 0; i < 70; ++i) {
      pmap.set_next_bit(false);
    }

    pmap.commit();

    BOOST_CHECK (byte_stream(sb) == byte_stream("\x80\x80\x40\x41\x42\xC3"));
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
                false,
                static_cast<const ascii_field_instruction*>(0));

    pmap.set_next_bit(true);
    for (std::size_t i = 0; i < 70; ++i) {
      pmap.set_next_bit(false);
    }

    pmap.commit();

    BOOST_CHECK (byte_stream(sb) == byte_stream("\x80\xC0\x40\x41\x42\xC3"));
  }
}



BOOST_AUTO_TEST_SUITE_END()
