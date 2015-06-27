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

#include <mfast/value_storage.h>
using namespace mfast;


using namespace mfast;

TEST_CASE("test the default construction of value_type", "[default_constructor_test]")
{
  value_storage value;

  REQUIRE(value.of_uint64.present_ ==                                 0U);
  REQUIRE(value.of_uint64.defined_bit_ ==                             0U);
  REQUIRE(value.of_uint64.content_ ==                               0ULL);

  REQUIRE(value.of_decimal.present_ ==                              0U);
  REQUIRE(value.of_decimal.exponent_ ==                              0);
  REQUIRE(value.of_decimal.defined_bit_ ==                          0U);
  REQUIRE(value.of_decimal.mantissa_ ==                            0LL);

  REQUIRE(value.of_group.present_ ==                                0U);
  REQUIRE(value.of_group.own_content_ ==                            0U);
  REQUIRE(value.of_group.defined_bit_ ==                            0U);
  REQUIRE(value.of_group.content_ ==                           nullptr);

  REQUIRE(value.of_array.len_ ==                                    0U);
  REQUIRE(value.of_array.capacity_in_bytes_ ==                      0U);
  REQUIRE(value.of_array.defined_bit_ ==                            0U);
  REQUIRE(value.of_array.content_ ==                           nullptr);

  REQUIRE(value.of_templateref.of_instruction.instruction_ ==  nullptr);
  REQUIRE(value.of_templateref.content_ ==                     nullptr);
}

TEST_CASE("test the value_storage constructed with 1", "[non_empty_constructor_test]")
{
  value_storage value(1);

  REQUIRE(value.of_uint64.present_ != 0);
  REQUIRE(value.of_uint64.defined_bit_ ==  1U);
  REQUIRE(value.of_uint64.content_ ==    0ULL);

  REQUIRE(value.of_decimal.present_ != 0);
  REQUIRE(value.of_decimal.exponent_ ==      0);
  REQUIRE(value.of_decimal.defined_bit_ ==  1U);
  REQUIRE(value.of_decimal.mantissa_ ==    0LL);

  REQUIRE(value.of_group.present_ != 0);
  REQUIRE(value.of_group.own_content_ ==   0U);
  REQUIRE(value.of_group.defined_bit_ ==   1U);
  REQUIRE(value.of_group.content_ ==  nullptr);

  REQUIRE(value.of_array.len_ != 0);
  REQUIRE(value.of_array.capacity_in_bytes_ ==  0U);
  REQUIRE(value.of_array.defined_bit_ ==        1U);
  REQUIRE(value.of_array.content_ ==       nullptr);
}

TEST_CASE("test value_storage.defined()", "[defined_test]")
{
  value_storage value;
  value.defined(true);
  REQUIRE(value.of_uint64.defined_bit_ ==     1U);
  REQUIRE(value.of_decimal.defined_bit_ ==  1U);
  REQUIRE(value.of_group.defined_bit_ ==    1U);
  REQUIRE(value.of_array.defined_bit_ ==    1U);

  value.defined(false);
  REQUIRE(value.of_uint64.defined_bit_ ==     0U);
  REQUIRE(value.of_decimal.defined_bit_ ==  0U);
  REQUIRE(value.of_group.defined_bit_ ==    0U);
  REQUIRE(value.of_array.defined_bit_ ==    0U);
}

TEST_CASE("test value_storage.present()","[empty_test]")
{
  value_storage value;

  value.present(true);
  REQUIRE(value.is_empty() ==  false);
  REQUIRE(value.of_uint64.present_ != 0);
  REQUIRE(value.of_decimal.present_ != 0);
  REQUIRE(value.of_group.present_ != 0);
  REQUIRE(value.of_array.len_ != 0);

  value.present(false);
  REQUIRE(value.is_empty() ==         true);
  REQUIRE(value.of_uint64.present_ ==     0U);
  REQUIRE(value.of_decimal.present_ ==  0U);
  REQUIRE(value.of_group.present_ ==    0U);
  REQUIRE(value.of_array.len_ ==        0U);
}

TEST_CASE("test value_storage.array_length","[array_length_test]")
{
  value_storage value;
  value.array_length(0);
  REQUIRE(value.is_empty() ==   false);
  REQUIRE(value.array_length() ==  0U);

  value.array_length(1);
  REQUIRE(value.is_empty() ==   false);
  REQUIRE(value.array_length() ==  1U);

  value.present(false);
  REQUIRE(value.is_empty() ==    true);
}

