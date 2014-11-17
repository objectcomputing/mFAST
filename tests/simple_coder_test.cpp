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
#include <mfast.h>
#include <mfast/field_comparator.h>
#include <mfast/coder/fast_encoder_v2.h>
#include <mfast/coder/fast_decoder_v2.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>
#include <stdexcept>

#include "simple1.h"
#include "simple2.h"
#include "simple3.h"
#include "simple4.h"
#include "simple5.h"
#include "simple6.h"
#include "simple7.h"

#include "byte_stream.h"
#include "debug_allocator.h"

using namespace mfast;

template <typename DESC>
class fast_coding_test_case
{
  public:
    fast_coding_test_case()
      : encoder_(DESC::instance(), &alloc_)
      , decoder_(DESC::instance(), &alloc_)
    {
    }

    boost::test_tools::predicate_result
    encoding(const message_cref& msg_ref, const byte_stream& result, bool reset=false)
    {
      const int buffer_size = 128;
      char buffer[buffer_size];

      std::size_t encoded_size = encoder_.encode(msg_ref,
                                                 buffer,
                                                 buffer_size,
                                                 reset);

      if (result == byte_stream(buffer, encoded_size))
        return true;

      boost::test_tools::predicate_result res( false );
      res.message() << "Got \"" << byte_stream(buffer, encoded_size) << "\" instead.";
      return res;
    }

    boost::test_tools::predicate_result
    decoding(const byte_stream& bytes, const message_cref& result, bool reset=false)
    {
      const char* first = bytes.data();
      message_cref msg = decoder_.decode(first, first+bytes.size(), reset);

      if (msg == result)
        return true;

      boost::test_tools::predicate_result res( false );
      return res;
    }

  private:
    debug_allocator alloc_;
    mfast::fast_encoder_v2 encoder_;
    mfast::fast_decoder_v2 decoder_;
};

BOOST_AUTO_TEST_SUITE( test_fast_coder2 )


BOOST_AUTO_TEST_CASE(simple_coder_test)
{
  fast_coding_test_case<simple1::templates_description> test_case;

  debug_allocator alloc;
  simple1::Test msg(&alloc);
  simple1::Test_mref msg_ref = msg.mref();

  msg_ref.set_field1().as(1);
  msg_ref.set_field2().as(2);
  msg_ref.set_field3().as(3);



  BOOST_CHECK(test_case.encoding(msg_ref,"\xB8\x81\x82\x83"));
  BOOST_CHECK(test_case.decoding("\xB8\x81\x82\x83", msg_ref));
}

BOOST_AUTO_TEST_CASE(group_coder_test)
{
  fast_coding_test_case<simple2::templates_description> test_case;

  debug_allocator alloc;
  simple2::Test msg(&alloc);
  simple2::Test_mref msg_ref = msg.mref();

  msg_ref.set_field1().as(1);
  msg_ref.set_group1().set_field2().as(2);
  msg_ref.set_group1().set_field3().as(3);

  BOOST_CHECK(test_case.encoding(msg_ref, "\xB0\x81\xE0\x82\x83"));
  BOOST_CHECK(test_case.decoding("\xB0\x81\xE0\x82\x83", msg_ref));

  // mfast::fast_decoder_v2< boost::mpl::vector<simple2::templates_description*> > decoder;
  // const char* strm = "\xB0\x81\xE0\x82\x83";
  //
  // simple2::Test_cref ref(decoder.decode(strm, strm +5));
  // BOOST_CHECK_EQUAL(ref.get_field1().value(), 1);
  // BOOST_CHECK(ref.get_group1().present());
  // BOOST_CHECK_EQUAL(ref.get_group1().get_field2().value(), 2);
  // BOOST_CHECK_EQUAL(ref.get_group1().get_field3().value(), 3);

}

BOOST_AUTO_TEST_CASE(sequence_coder_test)
{
  fast_coding_test_case<simple3::templates_description> test_case;

  debug_allocator alloc;
  simple3::Test msg(&alloc);
  simple3::Test_mref msg_ref = msg.mref();

  msg_ref.set_field1().as(1);
  simple3::Test_mref::sequence1_mref seq(msg_ref.set_sequence1());
  seq.resize(2);
  seq[0].set_field2().as(2);
  seq[0].set_field3().as(3);

  seq[1].set_field2().as(0);
  seq[1].set_field3().as(1);

  BOOST_CHECK(test_case.encoding(msg_ref,
                                 // pmap | f1 | s1 len| elem1 pmap | f2 | f3 | elem2 pmap |  f2 | f3 |
                                 //  A0    81    83        E0        82   83     E0          80   81
                                 "\xA0\x81\x83\xE0\x82\x83\xE0\x80\x81"
                                 ));
  BOOST_CHECK(test_case.decoding("\xA0\x81\x83\xE0\x82\x83\xE0\x80\x81", msg_ref));

}

BOOST_AUTO_TEST_CASE(static_templateref_coder_test)
{
  fast_coding_test_case<simple4::templates_description> test_case;

  debug_allocator alloc;
  simple4::Test msg(&alloc);
  simple4::Test_mref msg_ref = msg.mref();

  msg_ref.set_field1().as(1);
  msg_ref.set_field2().as(2);
  msg_ref.set_field3().as(3);
                                 // pmap | template id | field1 | field2 | field 3 |
                                 //  F8         82        81        82       83
  BOOST_CHECK(test_case.encoding(msg_ref,"\xF8\x82\x81\x82\x83"));
  BOOST_CHECK(test_case.decoding("\xF8\x82\x81\x82\x83", msg_ref));
}


BOOST_AUTO_TEST_CASE(dynamic_templateref_coder_test)
{
  fast_coding_test_case<simple5::templates_description> test_case;

  debug_allocator alloc;

  simple5::Test msg(&alloc);
  simple5::Test_mref msg_ref = msg.mref();


  msg_ref.set_field1().as(1);
  nested_message_mref nested(msg_ref.set_nested());

  simple5::Nested_mref target = nested.as<simple5::Nested>();

  target.set_field2().as(2);
  target.set_field3().as(3);

  BOOST_CHECK(test_case.encoding(msg_ref,"\xE0\x82\x81\xF0\x81\x82\x83"));
  BOOST_CHECK(test_case.decoding("\xE0\x82\x81\xF0\x81\x82\x83", msg_ref));
}

BOOST_AUTO_TEST_CASE(manual_reset_test)
{
  fast_coding_test_case<simple6::templates_description> test_case;

  debug_allocator alloc;
  simple6::Test msg(&alloc);
  simple6::Test_mref msg_ref = msg.mref();

  msg_ref.set_field1().as(1);
  msg_ref.set_field2().as(2);
  msg_ref.set_field3().as(3);

  BOOST_CHECK(test_case.encoding(msg_ref, "\xB8\x81\x82\x83", false));
  BOOST_CHECK(test_case.decoding("\xB8\x81\x82\x83", msg_ref, false));


  // message not changed
  BOOST_CHECK(test_case.encoding(msg_ref, "\x80", false));
  BOOST_CHECK(test_case.decoding("\x80", msg_ref, false));

  msg_ref.set_field1().as(11);
  msg_ref.set_field2().as(12);
  msg_ref.set_field3().as(13);

  BOOST_REQUIRE(!msg_ref.get_field1().absent());

  BOOST_REQUIRE(msg_ref.get_field1().is_initial_value());
  BOOST_REQUIRE(msg_ref.get_field2().is_initial_value());
  BOOST_REQUIRE(msg_ref.get_field3().is_initial_value());

  // encoding with reset, all values are initial
  BOOST_CHECK(test_case.encoding(msg_ref, "\x80", true));
  BOOST_CHECK(test_case.decoding("\x80", msg_ref, true));

}

BOOST_AUTO_TEST_CASE(auto_reset_coder_test)
{
  fast_coding_test_case<simple7::templates_description> test_case;

  debug_allocator alloc;
  simple7::Test msg(&alloc);
  simple7::Test_mref msg_ref = msg.mref();

  msg_ref.set_field1().as(1);
  msg_ref.set_field2().as(2);
  msg_ref.set_field3().as(3);

  // BOOST_CHECK(test_case.encoding(msg_ref, "\xB8\x81\x82\x83"));
  BOOST_CHECK(test_case.decoding("\xB8\x81\x82\x83", msg_ref));

  msg_ref.set_field1().as(11);
  msg_ref.set_field2().as(12);
  msg_ref.set_field3().as(13);
  // encoding with reset, all values are initial
  BOOST_CHECK(test_case.encoding(msg_ref, "\x80"));
  BOOST_CHECK(test_case.decoding("\x80", msg_ref));

  BOOST_CHECK(test_case.encoding(msg_ref, "\x80"));
  BOOST_CHECK(test_case.decoding("\x80", msg_ref));
}


BOOST_AUTO_TEST_SUITE_END()
