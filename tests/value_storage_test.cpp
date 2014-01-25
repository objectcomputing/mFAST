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

#include <mfast/value_storage.h>


#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>

using namespace mfast;

static const void* NULLPTR = 0;

BOOST_AUTO_TEST_SUITE( test_value_storage )

using namespace mfast;

BOOST_AUTO_TEST_CASE(default_constructor_test)
{
  value_storage value;

  BOOST_CHECK_EQUAL(value.of_uint.present_,                                0U);
  BOOST_CHECK_EQUAL(value.of_uint.defined_bit_,                            0U);
  BOOST_CHECK_EQUAL(value.of_uint.content_,                              0ULL);

  BOOST_CHECK_EQUAL(value.of_decimal.present_,                             0U);
  BOOST_CHECK_EQUAL(value.of_decimal.exponent_,                             0);
  BOOST_CHECK_EQUAL(value.of_decimal.defined_bit_,                         0U);
  BOOST_CHECK_EQUAL(value.of_decimal.mantissa_,                           0LL);

  BOOST_CHECK_EQUAL(value.of_group.present_,                               0U);
  BOOST_CHECK_EQUAL(value.of_group.own_content_,                           0U);
  BOOST_CHECK_EQUAL(value.of_group.defined_bit_,                           0U);
  BOOST_CHECK_EQUAL(value.of_group.content_,                          NULLPTR);

  BOOST_CHECK_EQUAL(value.of_array.len_,                                   0U);
  BOOST_CHECK_EQUAL(value.of_array.capacity_in_bytes_,                     0U);
  BOOST_CHECK_EQUAL(value.of_array.defined_bit_,                           0U);
  BOOST_CHECK_EQUAL(value.of_array.content_,                          NULLPTR);

  BOOST_CHECK_EQUAL(value.of_templateref.of_instruction.instruction_, NULLPTR);
  BOOST_CHECK_EQUAL(value.of_templateref.content_,                    NULLPTR);
}

BOOST_AUTO_TEST_CASE(non_empty_constructor_test)
{
  value_storage value(1);

  BOOST_CHECK(value.of_uint.present_ != 0);
  BOOST_CHECK_EQUAL(value.of_uint.defined_bit_, 1U);
  BOOST_CHECK_EQUAL(value.of_uint.content_,   0ULL);

  BOOST_CHECK(value.of_decimal.present_ != 0);
  BOOST_CHECK_EQUAL(value.of_decimal.exponent_,     0);
  BOOST_CHECK_EQUAL(value.of_decimal.defined_bit_, 1U);
  BOOST_CHECK_EQUAL(value.of_decimal.mantissa_,   0LL);

  BOOST_CHECK(value.of_group.present_ != 0);
  BOOST_CHECK_EQUAL(value.of_group.own_content_,  0U);
  BOOST_CHECK_EQUAL(value.of_group.defined_bit_,  1U);
  BOOST_CHECK_EQUAL(value.of_group.content_, NULLPTR);

  BOOST_CHECK(value.of_array.len_ != 0);
  BOOST_CHECK_EQUAL(value.of_array.capacity_in_bytes_, 0U);
  BOOST_CHECK_EQUAL(value.of_array.defined_bit_,       1U);
  BOOST_CHECK_EQUAL(value.of_array.content_,      NULLPTR);
}

BOOST_AUTO_TEST_CASE(defined_test)
{
  value_storage value;
  value.defined(true);
  BOOST_CHECK_EQUAL(value.of_uint.defined_bit_,    1U);
  BOOST_CHECK_EQUAL(value.of_decimal.defined_bit_, 1U);
  BOOST_CHECK_EQUAL(value.of_group.defined_bit_,   1U);
  BOOST_CHECK_EQUAL(value.of_array.defined_bit_,   1U);

  value.defined(false);
  BOOST_CHECK_EQUAL(value.of_uint.defined_bit_,    0U);
  BOOST_CHECK_EQUAL(value.of_decimal.defined_bit_, 0U);
  BOOST_CHECK_EQUAL(value.of_group.defined_bit_,   0U);
  BOOST_CHECK_EQUAL(value.of_array.defined_bit_,   0U);
}

BOOST_AUTO_TEST_CASE(empty_test)
{
  value_storage value;

  value.present(true);
  BOOST_CHECK_EQUAL(value.is_empty(), false);
  BOOST_CHECK(value.of_uint.present_ != 0);
  BOOST_CHECK(value.of_decimal.present_ != 0);
  BOOST_CHECK(value.of_group.present_ != 0);
  BOOST_CHECK(value.of_array.len_ != 0);

  value.present(false);
  BOOST_CHECK_EQUAL(value.is_empty(),        true);
  BOOST_CHECK_EQUAL(value.of_uint.present_,    0U);
  BOOST_CHECK_EQUAL(value.of_decimal.present_, 0U);
  BOOST_CHECK_EQUAL(value.of_group.present_,   0U);
  BOOST_CHECK_EQUAL(value.of_array.len_,       0U);
}

BOOST_AUTO_TEST_CASE(array_length_test)
{
  value_storage value;
  value.array_length(0);
  BOOST_CHECK_EQUAL(value.is_empty(),  false);
  BOOST_CHECK_EQUAL(value.array_length(), 0U);

  value.array_length(1);
  BOOST_CHECK_EQUAL(value.is_empty(),  false);
  BOOST_CHECK_EQUAL(value.array_length(), 1U);

  value.present(false);
  BOOST_CHECK_EQUAL(value.is_empty(),   true);
}

BOOST_AUTO_TEST_SUITE_END()
