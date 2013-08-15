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
#include <mfast/field_comparator.h>
#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <cstring>
#include "debug_allocator.h"
#include <stdexcept>

using namespace mfast;

BOOST_AUTO_TEST_SUITE( test_comparator )


BOOST_AUTO_TEST_CASE(simple_template_test)
{
  debug_allocator alloc;

  const char* xml_content =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy/></uInt32>\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy/></uInt32>\n"
    "<decimal name=\"field3\" id=\"13\"><copy/></decimal>\n"
    "<string name=\"field4\" id=\"14\"><copy/></string>\n"
    "</template>\n"
    "</templates>\n";
  dynamic_templates_description description(xml_content);

  message_type m1(&alloc, description.instruction(0));
  message_type m2(&alloc, description.instruction(0));

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref.mutable_field(0).as(0);
  m1ref.mutable_field(1).as(1);
  m1ref.mutable_field(2).as(2);
  m1ref.mutable_field(3).as("abcd");
  
  
  for (std::size_t i = 0; i < m1ref.fields_count(); ++i)
  {
    BOOST_CHECK_EQUAL(m1ref.const_field(i).instruction()->field_index(), i);
  }

  m2ref.mutable_field(0).as(0);
  m2ref.mutable_field(1).as(1);
  m2ref.mutable_field(2).as(2);
  m2ref.mutable_field(3).as("abcd");


  BOOST_CHECK(m1ref == m2ref);

  m2ref.mutable_field(2).as(3);
  BOOST_CHECK(m1ref != m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  BOOST_CHECK(m3.cref() == m1ref);
}

BOOST_AUTO_TEST_CASE(group_compare_test)
{
  debug_allocator alloc;

  const char* xml_content =
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
    "</templates>\n";

  dynamic_templates_description description(xml_content);

  message_type m1(&alloc, description.instruction(0));
  message_type m2(&alloc, description.instruction(0));

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref.mutable_field(0).as(0);
  group_mref m1group(m1ref.mutable_field(1));
  m1group.mutable_field(0).as(1);
  m1group.mutable_field(1).as(2);
  
  
  for (std::size_t i = 0; i < m1group.fields_count(); ++i)
  {
    BOOST_CHECK_EQUAL(m1group.const_field(i).instruction()->field_index(), i);
  }

  m2ref.mutable_field(0).as(0);
  group_mref m2group(m2ref.mutable_field(1));
  m2group.mutable_field(0).as(1);
  m2group.mutable_field(1).as(2);


  BOOST_CHECK(m1ref == m2ref);

  m2group.mutable_field(1).as(3);
  BOOST_CHECK(m1ref != m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  BOOST_CHECK(m3.cref() == m1ref);
}

BOOST_AUTO_TEST_CASE(sequence_compare_test)
{
  debug_allocator alloc;

  const char* xml_content =
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
    "</templates>\n";

  dynamic_templates_description description(xml_content);

  message_type m1(&alloc, description.instruction(0));
  message_type m2(&alloc, description.instruction(0));

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref.mutable_field(0).as(0);
  sequence_mref m1seq(m1ref.mutable_field(1));
  m1seq.resize(2);
  m1seq[0].mutable_field(0).as(1);
  m1seq[0].mutable_field(1).as(2);
  m1seq[1].mutable_field(0).as(3);
  m1seq[1].mutable_field(1).as(4);
  
  
  for (std::size_t i = 0; i < m1seq[0].fields_count(); ++i)
  {
    BOOST_CHECK_EQUAL(m1seq[0].const_field(i).instruction()->field_index(), i);
  }

  m2ref.mutable_field(0).as(0);
  sequence_mref m2seq(m2ref.mutable_field(1));
  m2seq.resize(2);
  m2seq[0].mutable_field(0).as(1);
  m2seq[0].mutable_field(1).as(2);
  m2seq[1].mutable_field(0).as(3);
  m2seq[1].mutable_field(1).as(4);

  BOOST_CHECK(m1ref == m2ref);

  m2seq[1].mutable_field(1).as(10);
  BOOST_CHECK(m1ref != m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);

  BOOST_CHECK(m3.cref() == m1ref);
}

BOOST_AUTO_TEST_CASE(static_templateref_compare_test)
{
  debug_allocator alloc;

  const char* xml_content =
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
    "</templates>\n";


  dynamic_templates_description description(xml_content);

  message_type m1(&alloc, description.instruction(1));
  message_type m2(&alloc, description.instruction(1));

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref.mutable_field(0).as(1);

  // static templateRef won't be resolved by dynamic_templates_description constructor
  // we have to manual resolve the reference manually.
  nested_message_mref nested1(m1ref.mutable_field(1));
  message_mref target1 = nested1.rebind(description.instruction(0));
  target1.mutable_field(0).as(2);
  target1.mutable_field(1).as(3);

  for (std::size_t i = 0; i < m1ref.fields_count(); ++i)
  {
    BOOST_CHECK_EQUAL(m1ref.const_field(i).instruction()->field_index(), i);
  }
  
  aggregate_mref aggregate1(nested1);
  BOOST_CHECK_EQUAL(aggregate1.instruction(), static_cast<const aggregate_instruction_base*>(target1.instruction()));

  m2ref.mutable_field(0).as(1);
  nested_message_mref nested2(m2ref.mutable_field(1));
  message_mref target2 = nested2.rebind(description.instruction(0));
  target2.mutable_field(0).as(2);
  target2.mutable_field(1).as(3);

  BOOST_CHECK(m1ref == m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  BOOST_CHECK(m3.cref() == m1ref);
}

BOOST_AUTO_TEST_CASE(dynamic_templateref_compare_test)
{
  debug_allocator alloc;

  const char* xml_content =
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
    "</templates>\n";


  dynamic_templates_description description(xml_content);

  message_type m1(&alloc, description.instruction(1));
  message_type m2(&alloc, description.instruction(1));

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref.mutable_field(0).as(1);

  nested_message_mref nested1(m1ref.mutable_field(1));
  BOOST_CHECK(!nested1.is_static());
  message_mref target1 = nested1.rebind(description.instruction(0));
  target1.mutable_field(0).as(2);
  target1.mutable_field(1).as(3);
  
  for (std::size_t i = 0; i < m1ref.fields_count(); ++i)
  {
    BOOST_CHECK_EQUAL(m1ref.const_field(i).instruction()->field_index(), i);
  } 


  m2ref.mutable_field(0).as(1);

  nested_message_mref nested2(m2ref.mutable_field(1));
  message_mref target2 = nested2.rebind(description.instruction(0));
  target2.mutable_field(0).as(2);
  target2.mutable_field(1).as(3);

  BOOST_CHECK(m1ref == m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  BOOST_CHECK(m3.cref() == m1ref);
}


BOOST_AUTO_TEST_SUITE_END()
