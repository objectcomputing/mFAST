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
#include <mfast.h>
#include <mfast/dynamic_templates_description.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>
#include "debug_allocator.h"
#include <stdexcept>

#include "byte_stream.h"
#include "debug_allocator.h"

using namespace mfast;

class message_encode_case
{
  public:
    message_encode_case(const char* xml_content)
      : description_(xml_content)
      , coder_(&alloc_)
    {
      const templates_description* descriptions[] = { &description_ };
      coder_.include(descriptions);
    }

    message_mref message()
    {
      return msg_.mref();
    }

    boost::test_tools::predicate_result
    assert_result(const byte_stream& result, bool reset=false)
    {
      const int buffer_size = 128;
      char buffer[buffer_size];

      std::size_t encoded_size = coder_.encode(msg_.cref(),
                                               buffer,
                                               buffer_size,
                                               reset);

      if (result == byte_stream(buffer, encoded_size))
        return true;

      boost::test_tools::predicate_result res( false );
      res.message() << "Got \"" << byte_stream(buffer, encoded_size) << "\" instead.";
      return res;
    }

    void set_template_id(uint32_t id)
    {
      msg_.rebind(&alloc_, coder_.template_with_id(id));
    }

    const template_instruction* template_with_id(uint32_t id)
    {
      return coder_.template_with_id(id);
    }

  private:
    dynamic_templates_description description_;
    debug_allocator alloc_;
    encoder coder_;
    message_base msg_;
};

BOOST_AUTO_TEST_SUITE( test_encoder )


BOOST_AUTO_TEST_CASE(simple_template_test)
{
  message_encode_case test_case (
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy/></uInt32>\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy/></uInt32>\n"
    "</template>\n"
    "</templates>\n");

  test_case.set_template_id(0);
  message_mref msg = test_case.message();

  msg.mutable_field(0).as(1);
  msg.mutable_field(1).as(2);
  msg.mutable_field(2).as(3);

  BOOST_CHECK(test_case.assert_result("\xB8\x81\x82\x83"));
}

BOOST_AUTO_TEST_CASE(group_test)
{
  message_encode_case test_case (
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy/></uInt32>\n"
    "<group name=\"group1\" presence=\"optional\">"
    "<uInt32 name=\"field2\" id=\"12\"><copy/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy/></uInt32>\n"
    "</group>"
    "</template>\n"
    "</templates>\n");

  test_case.set_template_id(0);
  message_mref msg = test_case.message();

  msg.mutable_field(0).as(1);
  group_mref grp = msg.mutable_field(1).static_cast_as<group_mref>();
  grp.mutable_field(0).as(2);
  grp.mutable_field(1).as(3);

  BOOST_CHECK(test_case.assert_result("\xB0\x81\xE0\x82\x83"));
}

BOOST_AUTO_TEST_CASE(sequence_test)
{
  message_encode_case test_case (
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy/></uInt32>\n"
    "<sequence name=\"sequence1\" presence=\"optional\">"
    "<uInt32 name=\"field2\" id=\"12\"><copy/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy/></uInt32>\n"
    "</sequence>"
    "</template>\n"
    "</templates>\n");

  test_case.set_template_id(0);
  message_mref msg = test_case.message();
  msg.mutable_field(0).as(1);
  sequence_mref seq = msg.mutable_field(1).static_cast_as<sequence_mref>();
  seq.resize(2);
  seq[0].mutable_field(0).as(2);
  seq[0].mutable_field(1).as(3);

  seq[1].mutable_field(0).as(0);
  seq[1].mutable_field(1).as(1);

  BOOST_CHECK(test_case.assert_result(
                // pmap | f1 | s1 len| elem1 pmap | f2 | f3 | elem2 pmap |  f2 | f3 |
                //  A0    81    83        E0        82   83     E0          80   81
                "\xA0\x81\x83\xE0\x82\x83\xE0\x80\x81"
                ));
}

BOOST_AUTO_TEST_CASE(static_templateref_test)
{
  message_encode_case test_case (
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Nested\" id=\"01\">\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy/></uInt32>\n"
    "</template>"
    "<template name=\"Test\" id=\"02\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy/></uInt32>\n"
    "<templateRef name=\"Nested\" />"
    "</template>\n"
    "</templates>\n");

  test_case.set_template_id(2);
  message_mref msg = test_case.message();
  msg.mutable_field(0).as(1);

  message_mref nested = msg.mutable_field(1).dynamic_cast_as<message_mref>();
  nested.mutable_field(0).as(2);
  nested.mutable_field(1).as(3);

  BOOST_CHECK(test_case.assert_result(
                "\xF8\x82\x81\x82\x83"
                ));
}

BOOST_AUTO_TEST_CASE(dynamic_templateref_test)
{
  message_encode_case test_case (
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Nested\" id=\"01\">\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy/></uInt32>\n"
    "</template>"
    "<template name=\"Test\" id=\"02\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy/></uInt32>\n"
    "<templateRef/>"
    "</template>\n"
    "</templates>\n");

  test_case.set_template_id(2);
  message_mref msg = test_case.message();
  msg.mutable_field(0).as(1);

  message_mref nested =  msg.mutable_field(1).static_cast_as<dynamic_mref>().rebind(test_case.template_with_id(1));
  nested.mutable_field(0).as(2);
  nested.mutable_field(1).as(3);

  BOOST_CHECK(test_case.assert_result(
                "\xE0\x82\x81\xF0\x81\x82\x83"
                ));

}

BOOST_AUTO_TEST_CASE(manual_reset_test)
{
  message_encode_case test_case (
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy value=\"11\"/></uInt32>\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy value=\"12\"/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy value=\"13\"/></uInt32>\n"
    "</template>\n"
    "</templates>\n");

  test_case.set_template_id(0);
  message_mref msg = test_case.message();
  msg.mutable_field(0).as(1);
  msg.mutable_field(1).as(2);
  msg.mutable_field(2).as(3);

  BOOST_CHECK(test_case.assert_result("\xB8\x81\x82\x83", false));
  
  // message not changed
  BOOST_CHECK(test_case.assert_result("\x80", false));
  
  msg.mutable_field(0).as(11);
  msg.mutable_field(1).as(12);
  msg.mutable_field(2).as(13);
  
  // encoding with reset, all values are initial 
  BOOST_CHECK(test_case.assert_result("\x80", true));
}

BOOST_AUTO_TEST_CASE(auto_reset_test)
{
  message_encode_case test_case (
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\" scp:reset=\"yes\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy value=\"11\"/></uInt32>\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy value=\"12\"/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy value=\"13\"/></uInt32>\n"
    "</template>\n"
    "</templates>\n");

    test_case.set_template_id(0);
    message_mref msg = test_case.message();
    msg.mutable_field(0).as(1);
    msg.mutable_field(1).as(2);
    msg.mutable_field(2).as(3);

    BOOST_CHECK(test_case.assert_result("\xB8\x81\x82\x83"));
    
    msg.mutable_field(0).as(11);
    msg.mutable_field(1).as(12);
    msg.mutable_field(2).as(13);  
    // encoding with reset, all values are initial 
    BOOST_CHECK(test_case.assert_result("\x80"));
  
    BOOST_CHECK(test_case.assert_result("\x80"));
}


BOOST_AUTO_TEST_SUITE_END()
