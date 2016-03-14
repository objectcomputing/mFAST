// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.

#include "catch.hpp"
#include "debug_allocator.h"

#include "test1.h"
#include "test2.h"


bool
equal_string(const char* value, const char* str)
{
  if (std::strcmp(value, str) == 0) {
    return true;
  }
  INFO( "value was \"" << value << "\"" );
  return false;
}

template <typename T>
bool
equal_string(T value, const char* str)
{
  if (std::strcmp(value.c_str(), str) == 0) {
    return true;
  }
  INFO("value was \"" << value.c_str() << "\"");
  return false;
}


TEST_CASE("test the operations of generated code","[MDRefreshSample_test]")
{
  debug_allocator alloc;
  test1::SampleInfo info(&alloc);
  REQUIRE((int)test1::SampleInfo::the_id ==                              1);
  REQUIRE(test1::SampleInfo::instruction()->id() ==                     1U);
  REQUIRE(test1::SampleInfo::instruction()->subinstructions().size() ==  4U);
  test1::SampleInfo_cref info_cref = info.cref();

  REQUIRE(equal_string(info_cref.get_BeginString(), "FIX4.4"));
  REQUIRE(info_cref.get_BeginString().instruction()->field_type() ==      mfast::field_type_ascii_string);
  REQUIRE(info_cref.get_BeginString().instruction()->id() ==                  8U);
  REQUIRE(info_cref.get_BeginString().instruction()->field_operator() ==  mfast::operator_constant);

  REQUIRE(equal_string(info_cref.get_MessageType(), "X"));
  REQUIRE(info_cref.get_MessageType().instruction()->field_type() ==           mfast::field_type_ascii_string);
  REQUIRE(info_cref.get_MessageType().instruction()->id() ==                      35U);
  REQUIRE(info_cref.get_MessageType().instruction()->field_operator() ==       mfast::operator_constant);

  REQUIRE(info_cref.get_SenderCompID().instruction()->field_type() ==          mfast::field_type_ascii_string);
  REQUIRE(info_cref.get_SenderCompID().instruction()->id() ==                     49U);
  REQUIRE(info_cref.get_SenderCompID().instruction()->field_operator() ==      mfast::operator_copy);

  REQUIRE(info_cref.get_MsgSeqNum().instruction()->field_type() ==             mfast::field_type_uint32);
  REQUIRE(info_cref.get_MsgSeqNum().instruction()->id() ==                        34U);
  REQUIRE(info_cref.get_MsgSeqNum().instruction()->field_operator() ==         mfast::operator_increment);

  test2::MDRefreshSample sample(&alloc);
  REQUIRE((int)test2::MDRefreshSample::the_id ==                                   2);
  REQUIRE(test2::MDRefreshSample::instruction()->id() ==                     2U);
  REQUIRE(test2::MDRefreshSample::instruction()->subinstructions().size() ==  3U);

  test2::MDRefreshSample::cref_type sample_cref = sample.cref();
  test1::SampleInfo_cref si = sample_cref.get_info();
  // REQUIRE(si.present() ==  true);
  REQUIRE(si.instruction()->segment_pmap_size() ==  2U);

  REQUIRE(sample.cref().get_MDEntries().instruction()->field_type() ==         mfast::field_type_sequence);
  // REQUIRE(sample.cref().get_MDEntries().instruction()->subinstructions().size() ==  11U);

  const mfast::uint32_field_instruction* len_inst = sample.cref().get_MDEntries().instruction()->length_instruction();

  REQUIRE(len_inst != nullptr);
  if (len_inst) {
	REQUIRE(equal_string(len_inst->name(), "NoMDEntries"));
	REQUIRE(len_inst->id() == 268U);
  }

  sample.mref().set_MDEntries().resize(1);
  typedef test2::MDRefreshSample_cref::MDEntries_element_cref MDEntries_element_cref;
  MDEntries_element_cref elem0 = sample.cref().get_MDEntries()[0];

  REQUIRE(elem0.get_MDUpdateAction().instruction()->id() ==                279U);
  REQUIRE(elem0.get_MDUpdateAction().instruction()->field_type() ==      mfast::field_type_uint32);
  REQUIRE(elem0.get_MDUpdateAction().instruction()->field_operator() ==  mfast::operator_copy);

  REQUIRE(elem0.get_MDEntrySize().instruction()->id() ==                   271U);
  REQUIRE(elem0.get_MDEntrySize().instruction()->field_type() ==         mfast::field_type_exponent);
  REQUIRE(elem0.get_MDEntrySize().instruction()->field_operator() ==     mfast::operator_copy);

  const mfast::mantissa_field_instruction* mantissa_inst = elem0.get_MDEntrySize().instruction()->mantissa_instruction();
  REQUIRE(mantissa_inst->field_type() ==                                 mfast::field_type_int64);
  REQUIRE(mantissa_inst->field_operator() ==                             mfast::operator_delta);

  sample.mref().set_MDEntries().resize(2);
  sample.mref().set_MDEntries().resize(4);

  MDEntries_element_cref elem3 = sample.cref().get_MDEntries()[3];
  REQUIRE(elem3.get_MDEntryType().size() ==            0U);

  REQUIRE(sample_cref.instruction()->subinstructions().size() ==  3U);
  REQUIRE(sample_cref.instruction()->subinstruction(2)->field_type() ==  mfast::field_type_sequence);


  test2::MDRefreshSample_mref::extra_mref extra_mref =  sample.mref().set_extra();

  REQUIRE(extra_mref.instruction() ==  sample_cref.instruction()->subinstruction(2));

  extra_mref.resize(1);
  REQUIRE(extra_mref.instruction() ==  sample_cref.instruction()->subinstruction(2));
  REQUIRE(extra_mref.instruction()->element_instruction()->field_type() ==  mfast::field_type_template);

  test1::SampleInfo_mref extra0 (extra_mref[0]);

  REQUIRE(equal_string(extra0.get_BeginString(), "FIX4.4"));

  mfast::message_cref generic_cref(sample.cref());
  test2::MDRefreshSample::cref_type specific_cref(generic_cref);

  mfast::message_mref generic_mref(sample.mref());
  test2::MDRefreshSample::mref_type specific_mref(generic_mref);


}

