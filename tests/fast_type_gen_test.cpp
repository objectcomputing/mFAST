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
#include "test1.h"
#include "test2.h"
#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "debug_allocator.h"

boost::test_tools::predicate_result
equal_string(const char* value, const char* str)
{
  if (std::strcmp(value, str) == 0) {
    return true;
  }
  boost::test_tools::predicate_result res( false );
  res.message() << "value was \"" << value << "\"";
  return res;
}

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
  BOOST_CHECK_EQUAL((int)test1::SampleInfo::the_id,                             1);
  BOOST_CHECK_EQUAL(test1::SampleInfo::instruction()->id(),                    1U);
  BOOST_CHECK_EQUAL(test1::SampleInfo::instruction()->subinstructions().size(), 4U);
  test1::SampleInfo_cref info_cref = info.cref();

  BOOST_CHECK(equal_string(info_cref.get_BeginString(), "FIX4.4"));
  BOOST_CHECK_EQUAL(info_cref.get_BeginString().instruction()->field_type(),     mfast::field_type_ascii_string);
  BOOST_CHECK_EQUAL(info_cref.get_BeginString().instruction()->id(),                 8U);
  BOOST_CHECK_EQUAL(info_cref.get_BeginString().instruction()->field_operator(), mfast::operator_constant);

  BOOST_CHECK(equal_string(info_cref.get_MessageType(), "X"));
  BOOST_CHECK_EQUAL(info_cref.get_MessageType().instruction()->field_type(),          mfast::field_type_ascii_string);
  BOOST_CHECK_EQUAL(info_cref.get_MessageType().instruction()->id(),                     35U);
  BOOST_CHECK_EQUAL(info_cref.get_MessageType().instruction()->field_operator(),      mfast::operator_constant);

  BOOST_CHECK_EQUAL(info_cref.get_SenderCompID().instruction()->field_type(),         mfast::field_type_ascii_string);
  BOOST_CHECK_EQUAL(info_cref.get_SenderCompID().instruction()->id(),                    49U);
  BOOST_CHECK_EQUAL(info_cref.get_SenderCompID().instruction()->field_operator(),     mfast::operator_copy);

  BOOST_CHECK_EQUAL(info_cref.get_MsgSeqNum().instruction()->field_type(),            mfast::field_type_uint32);
  BOOST_CHECK_EQUAL(info_cref.get_MsgSeqNum().instruction()->id(),                       34U);
  BOOST_CHECK_EQUAL(info_cref.get_MsgSeqNum().instruction()->field_operator(),        mfast::operator_increment);

  test2::MDRefreshSample sample(&alloc);
  BOOST_CHECK_EQUAL((int)test2::MDRefreshSample::the_id,                                  2);
  BOOST_CHECK_EQUAL(test2::MDRefreshSample::instruction()->id(),                    2U);
  BOOST_CHECK_EQUAL(test2::MDRefreshSample::instruction()->subinstructions().size(), 3U);

  test2::MDRefreshSample::cref_type sample_cref = sample.cref();
  test1::SampleInfo_cref si = sample_cref.get_info();
  // BOOST_CHECK_EQUAL(si.present(), true);
  BOOST_CHECK_EQUAL(si.instruction()->segment_pmap_size(), 2U);

  BOOST_CHECK_EQUAL(sample.cref().get_MDEntries().instruction()->field_type(),        mfast::field_type_sequence);
  // BOOST_CHECK_EQUAL(sample.cref().get_MDEntries().instruction()->subinstructions().size(), 11U);

  const mfast::uint32_field_instruction* len_inst = sample.cref().get_MDEntries().instruction()->length_instruction();

  BOOST_CHECK(len_inst != 0);
  if (len_inst) {
	BOOST_CHECK(equal_string(len_inst->name(), "NoMDEntries"));
	BOOST_CHECK_EQUAL(len_inst->id(),268U);
  }

  sample.mref().set_MDEntries().resize(1);
  typedef test2::MDRefreshSample_cref::MDEntries_element_cref MDEntries_element_cref;
  MDEntries_element_cref elem0 = sample.cref().get_MDEntries()[0];

  BOOST_CHECK_EQUAL(elem0.get_MDUpdateAction().instruction()->id(),               279U);
  BOOST_CHECK_EQUAL(elem0.get_MDUpdateAction().instruction()->field_type(),     mfast::field_type_uint32);
  BOOST_CHECK_EQUAL(elem0.get_MDUpdateAction().instruction()->field_operator(), mfast::operator_copy);

  BOOST_CHECK_EQUAL(elem0.get_MDEntrySize().instruction()->id(),                  271U);
  BOOST_CHECK_EQUAL(elem0.get_MDEntrySize().instruction()->field_type(),        mfast::field_type_exponent);
  BOOST_CHECK_EQUAL(elem0.get_MDEntrySize().instruction()->field_operator(),    mfast::operator_copy);

  const mfast::mantissa_field_instruction* mantissa_inst = elem0.get_MDEntrySize().instruction()->mantissa_instruction();
  BOOST_CHECK_EQUAL(mantissa_inst->field_type(),                                mfast::field_type_int64);
  BOOST_CHECK_EQUAL(mantissa_inst->field_operator(),                            mfast::operator_delta);

  sample.mref().set_MDEntries().resize(2);
  sample.mref().set_MDEntries().resize(4);

  MDEntries_element_cref elem3 = sample.cref().get_MDEntries()[3];
  BOOST_CHECK_EQUAL(elem3.get_MDEntryType().size(),           0U);

  BOOST_CHECK_EQUAL(sample_cref.instruction()->subinstructions().size(), 3U);
  BOOST_CHECK_EQUAL(sample_cref.instruction()->subinstruction(2)->field_type(), mfast::field_type_sequence);


  test2::MDRefreshSample_mref::extra_mref extra_mref =  sample.mref().set_extra();

  BOOST_CHECK_EQUAL(extra_mref.instruction(), sample_cref.instruction()->subinstruction(2));

  extra_mref.resize(1);
  BOOST_CHECK_EQUAL(extra_mref.instruction(), sample_cref.instruction()->subinstruction(2));
  BOOST_CHECK_EQUAL(extra_mref.instruction()->element_instruction()->field_type(), mfast::field_type_template);

  test1::SampleInfo_mref extra0 (extra_mref[0]);

  BOOST_CHECK(equal_string(extra0.get_BeginString(), "FIX4.4"));

  mfast::message_cref generic_cref(sample.cref());
  test2::MDRefreshSample::cref_type specific_cref(generic_cref);

  mfast::message_mref generic_mref(sample.mref());
  test2::MDRefreshSample::mref_type specific_mref(generic_mref);


}

BOOST_AUTO_TEST_SUITE_END()
