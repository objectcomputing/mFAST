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
#include <mfast/fast_istream_extractor.h>
#include <boost/test/test_tools.hpp>

#include <boost/test/unit_test.hpp>

#include "debug_allocator.h"
#include <stdexcept>

using namespace mfast;

BOOST_AUTO_TEST_SUITE( fast_istream_test_suit )


BOOST_AUTO_TEST_CASE(int32_test)
{
  {
    char data[] = "\x39\x45\xa4";
    fast_istream strm(data, 3);

    int32_t value;
    BOOST_CHECK(strm.decode(value,true));
    BOOST_CHECK_EQUAL(value, 942755);
  }

  {
    char data[] = "\x39\x45\xa3";
    fast_istream strm(data, 3);

    int32_t value;
    BOOST_CHECK(strm.decode(value,false));
    BOOST_CHECK_EQUAL(value, 942755);
  }

  {
    char data[] = "\x46\x3a\xdd";
    fast_istream strm(data, 3);

    int32_t value;
    BOOST_CHECK(strm.decode(value,true));
    BOOST_CHECK_EQUAL(value, -942755);
  }

  {
    char data[] = "\x7c\x1b\x1b\x9d";
    fast_istream strm(data, 4);

    int32_t value;
    BOOST_CHECK(strm.decode(value,false));
    BOOST_CHECK_EQUAL(value, -7942755);
  }

  {
    char data[] = "\x00\x40\x81";
    fast_istream strm(data, 3);

    int32_t value;
    BOOST_CHECK(strm.decode(value,false));
    BOOST_CHECK_EQUAL(value, 8193);
  }

  {
    char data[] = "\x7F\x3f\xff";
    fast_istream strm(data, 4);

    int32_t value;
    BOOST_CHECK(strm.decode(value,false));
    BOOST_CHECK_EQUAL(value, -8193);
  }

}

BOOST_AUTO_TEST_CASE(uint32_test)
{
  {
    char data[] = "\x80";
    fast_istream strm(data, 3);

    uint32_t value;
    BOOST_CHECK(!strm.decode(value,true));
  }
  {
    char data[] = "\x81";
    fast_istream strm(data, 3);

    uint32_t value;
    BOOST_CHECK(strm.decode(value,true));
    BOOST_CHECK_EQUAL(value, 0);
  }

  {
    char data[] = "\x82";
    fast_istream strm(data, 3);

    uint32_t value;
    BOOST_CHECK(strm.decode(value,true));
    BOOST_CHECK_EQUAL(value, 1);
  }

  {
    char data[] = "\x39\x45\xa4";
    fast_istream strm(data, 3);

    uint32_t value;
    BOOST_CHECK(strm.decode(value,true));
    BOOST_CHECK_EQUAL(value, 942755);
  }

  {
    char data[] = "\x80";
    fast_istream strm(data, 3);

    uint32_t value;
    BOOST_CHECK(strm.decode(value,false));
    BOOST_CHECK_EQUAL(value, 0);

  }
  {
    char data[] = "\x81";
    fast_istream strm(data, 3);

    uint32_t value;
    BOOST_CHECK(strm.decode(value,false));
    BOOST_CHECK_EQUAL(value, 1);
  }


  {
    char data[] = "\x39\x45\xa3";
    fast_istream strm(data, 3);

    uint32_t value;
    BOOST_CHECK(strm.decode(value,false));
    BOOST_CHECK_EQUAL(value, 942755);
  }

  {
    char data[] = "\x10\x00\x00\x00\x80";
    fast_istream strm(data, 5);

    uint32_t value;
    BOOST_CHECK(strm.decode(value,true));
    BOOST_CHECK_EQUAL(value, 4294967295);
  }
}

BOOST_AUTO_TEST_CASE(uint64_test)
{
  {
    char data[] = "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80";
    fast_istream strm(data, 10);

    uint64_t value;
    BOOST_CHECK (strm.decode(value,true));
    BOOST_CHECK_EQUAL(value, std::numeric_limits<uint64_t>::max());
  }

}


BOOST_AUTO_TEST_CASE(ascii_string_test)
{
  ascii_field_instruction* instruction = 0;
  const char* str;
  uint32_t len;
  {
    char data[] = "\x80";
    fast_istream strm(data, 1);
    BOOST_CHECK( strm.decode(str, len, false, instruction) );
    BOOST_CHECK_EQUAL( len,             0);
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }

  {
    char data[] = "\x00\x80";
    fast_istream strm(data, 2);
    BOOST_CHECK( strm.decode(str, len, false, instruction) );
    BOOST_CHECK_EQUAL( len,             1);
    BOOST_CHECK_EQUAL( str[0],      '\x0');
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }

  {
    char data[] = "\x00\xC0";
    fast_istream strm(data, 2);
    BOOST_CHECK_THROW( strm.decode(str, len, false, instruction), mfast::fast_error );
  }

  {
    char data[] = "\x80";
    fast_istream strm(data, 1);
    BOOST_CHECK( !strm.decode(str, len, true, instruction) );
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }

  {
    char data[] = "\x00\x80";
    fast_istream strm(data, 2);
    BOOST_CHECK( strm.decode(str, len, true, instruction) );
    BOOST_CHECK_EQUAL( len,             0);
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }

  {
    char data[] = "\x00\xC0";
    fast_istream strm(data, 2);
    BOOST_CHECK_THROW( strm.decode(str, len, true, instruction), mfast::fast_error );
  }

  {
    char data[] = "\x00\x00\x80";
    fast_istream strm(data, 3);
    BOOST_CHECK( strm.decode(str, len, true, instruction) );
    BOOST_CHECK_EQUAL( len,             1);
    BOOST_CHECK_EQUAL( str[0],      '\x0');
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }

  {
    char data[] = "\x00\x00\xC0";
    fast_istream strm(data, 3);
    BOOST_CHECK_THROW( strm.decode(str, len, true, instruction), mfast::fast_error );
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }

  {
    char data[] = "\x40\x40\xC0";
    fast_istream strm(data, 3);
    BOOST_CHECK( strm.decode(str, len, true, instruction) );

    BOOST_CHECK_EQUAL( len,             3);
    BOOST_CHECK_EQUAL( str,          data);
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }
  {
    char data[] = "\x40\x40\xC0";
    fast_istream strm(data, 3);
    BOOST_CHECK( strm.decode(str, len, false, instruction) );
    BOOST_CHECK_EQUAL( len,             3);
    BOOST_CHECK_EQUAL( str,          data);
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }
}

BOOST_AUTO_TEST_CASE(byte_vector_test)
{
  const unsigned char* str;
  uint32_t len;
  {
    char data[] = "\x80";
    fast_istream strm(data, 1);
    BOOST_CHECK(!strm.decode(str, len, true, 0));
  }
  {
    // empty byte vector
    char data[] = "\x80";
    fast_istream strm(data, 1);
    BOOST_CHECK(strm.decode(str, len, false, 0));
    BOOST_CHECK_EQUAL( len,             0);
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);

  }
  {
    // empty byte vector
    char data[] = "\x81";
    fast_istream strm(data, 1);
    BOOST_CHECK(strm.decode(str, len, true, 0));
    BOOST_CHECK_EQUAL( len,             0);
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }
  {
    char data[] = "\x81\xC0";
    fast_istream strm(data, 2);
    BOOST_CHECK(strm.decode(str, len, false, 0));
    BOOST_CHECK_EQUAL( len,                                   1);
    BOOST_CHECK_EQUAL( reinterpret_cast<const char*>(str), data + 1);
    BOOST_CHECK_EQUAL( strm.in_avail(),                       0);
  }
  {
    char data[] = "\x82\xC0";
    fast_istream strm(data, 2);
    BOOST_CHECK(strm.decode(str, len, true, 0));
    BOOST_CHECK_EQUAL( len,                                   1);
    BOOST_CHECK_EQUAL( reinterpret_cast<const char*>(str), data + 1);
    BOOST_CHECK_EQUAL( strm.in_avail(),                       0);
  }
}


BOOST_AUTO_TEST_CASE(extractor_test)
{
  debug_allocator alloc;
  value_storage_t storage;
  {
    const char* default_value = "initial_string";
    ascii_field_instruction inst(operator_copy,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 default_value, strlen(default_value));

    inst.construct_value(storage, &alloc);
    ascii_string_mref mref(&alloc, &storage, &inst);

    char data[] = "\x41\x41\xC1";
    fast_istream strm(data, 3);
    strm >> mref;
    BOOST_CHECK_EQUAL( mref.size(), 3);
    BOOST_CHECK( mref ==  "AAA");
    BOOST_CHECK_EQUAL( strm.in_avail(), 0);
  }

  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0,
                                   nullable_decimal(INT64_MAX,64));

    inst.construct_value(storage, &alloc);
    decimal_mref mref(&alloc, &storage, &inst);


    {
      char data[] = "\x80";
      fast_istream strm(data, 1);
      strm >> mref;

      BOOST_CHECK( mref.absent() );
      BOOST_CHECK_EQUAL( strm.in_avail(), 0);
    }

    {
      char data[] = "\x85\x81";
      fast_istream strm(data, 2);
      strm >> mref;

      BOOST_CHECK( mref.present() );
      BOOST_CHECK_EQUAL( mref.mantissa(), 1 );
      BOOST_CHECK_EQUAL( mref.exponent(), 4 );
      BOOST_CHECK_EQUAL( strm.in_avail(), 0);
    }

  }

}

BOOST_AUTO_TEST_CASE(presence_map_test)
{
  {
    char data[]= "\xC0";
    fast_istream strm(data, 1);
    debug_allocator alloc;

    presence_map pmap;
    strm.decode(pmap);

    BOOST_CHECK_EQUAL(strm.in_avail(), 0);
      BOOST_CHECK(pmap.is_next_bit_set());

    for (int i =0; i < 8; ++i) {
      BOOST_CHECK(!pmap.is_next_bit_set());
    }


    {
      char data[]= "\x40\x81";
      fast_istream strm(data, 2);

      presence_map pmap;
      strm.decode(pmap);

      BOOST_CHECK_EQUAL(strm.in_avail(), 0);

        BOOST_CHECK(pmap.is_next_bit_set());

      for (int i =0; i < 12; ++i) {
        BOOST_CHECK(!pmap.is_next_bit_set());
      }

        BOOST_CHECK(pmap.is_next_bit_set());

      for (int i =0; i < 7; ++i) {
        BOOST_CHECK(!pmap.is_next_bit_set());
      }
    }

    {
      char buf[16];
      // find the address in buf which aligned in the 8 bytes boundary

      char* aligned_buf = reinterpret_cast<char*> (reinterpret_cast<uintptr_t>(buf + 8) & (~(uintptr_t)7));
      char* data = aligned_buf -1;
      data[0] = '\x40';
      data[1] = '\x81';

      fast_istream strm(data, 2);

      presence_map pmap;
      strm.decode(pmap);

      BOOST_CHECK_EQUAL(strm.in_avail(), 0);

        BOOST_CHECK(pmap.is_next_bit_set());

      for (int i =0; i < 12; ++i) {
        BOOST_CHECK(!pmap.is_next_bit_set());
      }

        BOOST_CHECK(pmap.is_next_bit_set());

      for (int i =0; i < 7; ++i) {
        BOOST_CHECK(!pmap.is_next_bit_set());
      }
    }

    {
      char data[]= "\x40\x40\x40\x40\x40\x40\x40\x40\xC0";
      fast_istream strm(data, 9);

      presence_map pmap;
      strm.decode(pmap);

      BOOST_CHECK_EQUAL(strm.in_avail(), 0);

      for (int i = 0; i < 9; ++i) {
          BOOST_CHECK(pmap.is_next_bit_set());
        for (int j = 0; j < 6; ++j) {
          BOOST_CHECK(!pmap.is_next_bit_set());
        }
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
