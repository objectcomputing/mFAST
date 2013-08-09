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
#include "test1.h"
#include "test2.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "debug_allocator.h"


template <typename T>
boost::test_tools::predicate_result
equal_string(T value, const char* str)
{
  if (std::strcmp(value.c_str(), str) == 0) {
    return true;
  }
  boost::test_tools::predicate_result res( false );
  res.message() << "value was \"" << value.c_str() << "\"";
  return res;
}

BOOST_AUTO_TEST_SUITE( fast_type_gen_test_suite )

BOOST_AUTO_TEST_CASE(MDRefreshSample_test)
{
  debug_allocator alloc;
  test1::SampleInfo info(&alloc);
  BOOST_CHECK_EQUAL((int)test1::SampleInfo::the_id,                                  1);
  BOOST_CHECK_EQUAL(test1::SampleInfo_cref::the_instruction.id(),                    1);
  BOOST_CHECK_EQUAL(test1::SampleInfo_cref::the_instruction.subinstructions_count(), 4);
  test1::SampleInfo_cref info_cref = info.cref();
  
  BOOST_CHECK(equal_string(info_cref.get_BeginString(), "FIX4.4"));
  BOOST_CHECK_EQUAL(info_cref.get_BeginString().instruction()->field_index(),        0);
  BOOST_CHECK_EQUAL(info_cref.get_BeginString().instruction()->field_type(),     mfast::field_type_ascii_string);
  BOOST_CHECK_EQUAL(info_cref.get_BeginString().instruction()->id(),                 8);
  BOOST_CHECK_EQUAL(info_cref.get_BeginString().instruction()->field_operator(), mfast::operator_constant);
  
  BOOST_CHECK(equal_string(info_cref.get_MessageType(), "X"));
  BOOST_CHECK_EQUAL(info_cref.get_MessageType().instruction()->field_index(),             1);
  BOOST_CHECK_EQUAL(info_cref.get_MessageType().instruction()->field_type(),          mfast::field_type_ascii_string);
  BOOST_CHECK_EQUAL(info_cref.get_MessageType().instruction()->id(),                     35);
  BOOST_CHECK_EQUAL(info_cref.get_MessageType().instruction()->field_operator(),      mfast::operator_constant);
  
  BOOST_CHECK_EQUAL(info_cref.get_SenderCompID().instruction()->field_index(),            2);
  BOOST_CHECK_EQUAL(info_cref.get_SenderCompID().instruction()->field_type(),         mfast::field_type_ascii_string);
  BOOST_CHECK_EQUAL(info_cref.get_SenderCompID().instruction()->id(),                    49);
  BOOST_CHECK_EQUAL(info_cref.get_SenderCompID().instruction()->field_operator(),     mfast::operator_copy);
  
  BOOST_CHECK_EQUAL(info_cref.get_MsgSeqNum().instruction()->field_index(),               3);
  BOOST_CHECK_EQUAL(info_cref.get_MsgSeqNum().instruction()->field_type(),            mfast::field_type_uint32);
  BOOST_CHECK_EQUAL(info_cref.get_MsgSeqNum().instruction()->id(),                       34);
  BOOST_CHECK_EQUAL(info_cref.get_MsgSeqNum().instruction()->field_operator(),        mfast::operator_increment);

  test2::MDRefreshSample sample(&alloc);
  BOOST_CHECK_EQUAL((int)test2::MDRefreshSample::the_id,                                  2);
  BOOST_CHECK_EQUAL(test2::MDRefreshSample_cref::the_instruction.id(),                    2);
  BOOST_CHECK_EQUAL(test2::MDRefreshSample_cref::the_instruction.subinstructions_count(), 3);
  
  test2::MDRefreshSample_cref sample_cref = sample.cref();
  test1::SampleInfo_cref si = sample_cref.get_SampleInfo();
  BOOST_CHECK_EQUAL(si.instruction(),   &test1::SampleInfo_cref::the_instruction);
  BOOST_CHECK_EQUAL(si.instruction()->segment_pmap_size(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
