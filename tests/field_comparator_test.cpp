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

#include <mfast.h>
#include <mfast/field_comparator.h>
#include <mfast/xml_parser/dynamic_templates_description.h>

#include <cstring>
#include "debug_allocator.h"
#include <stdexcept>

using namespace mfast;



TEST_CASE("test the == operator of a simple message", "[simple_template_test]")
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

  message_type m1(&alloc, description[0]);
  message_type m2(&alloc, description[0]);

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref[0].as(0);
  m1ref[1].as(1);
  m1ref[2].as(2);
  m1ref[3].as("abcd");

  m2ref[0].as(0);
  m2ref[1].as(1);
  m2ref[2].as(2);
  m2ref[3].as("abcd");

  REQUIRE(m1ref[0] == m2ref[0]);
  REQUIRE(m1ref == m2ref);

  m2ref[2].as(3);
  REQUIRE_FALSE(m1ref == m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  REQUIRE(m3.cref() == m1ref);
}

TEST_CASE("test the == operator of a group","[group_compare_test]")
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

  message_type m1(&alloc, description[0]);
  message_type m2(&alloc, description[0]);

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref[0].as(0);
  group_mref m1group(m1ref[1]);
  m1group[0].as(1);
  m1group[1].as(2);


  m2ref[0].as(0);
  group_mref m2group(m2ref[1]);
  m2group[0].as(1);
  m2group[1].as(2);


  REQUIRE(m1ref == m2ref);

  m2group[1].as(3);
  REQUIRE_FALSE(m1ref == m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  REQUIRE(m3.cref() == m1ref);
}

TEST_CASE("test the == operator of sequence","[sequence_compare_test]")
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

  message_type m1(&alloc, description[0]);
  message_type m2(&alloc, description[0]);

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref[0].as(0);
  sequence_mref m1seq(m1ref[1]);
  m1seq.resize(2);
  m1seq[0][0].as(1);
  m1seq[0][1].as(2);
  m1seq[1][0].as(3);
  m1seq[1][1].as(4);

  m2ref[0].as(0);
  sequence_mref m2seq(m2ref[1]);
  m2seq.resize(2);
  m2seq[0][0].as(1);
  m2seq[0][1].as(2);
  m2seq[1][0].as(3);
  m2seq[1][1].as(4);

  REQUIRE(m1ref == m2ref);

  m2seq[1][1].as(10);
  REQUIRE_FALSE(m1ref == m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);

  REQUIRE(m3.cref() == m1ref);
}

TEST_CASE("test the == operator of static templateref","[static_templateref_compare_test]")
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

  message_type m1(&alloc, description[1]);
  message_type m2(&alloc, description[1]);

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref[0].as(1);
  m1ref[1].as(2);
  m1ref[2].as(3);

  m2ref[0].as(1);
  m2ref[1].as(2);
  m2ref[2].as(3);

  REQUIRE(m1ref == m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  REQUIRE(m3.cref() == m1ref);
}

TEST_CASE("test the == operator of dynamic templateref","[dynamic_templateref_compare_test]")
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

  message_type m1(&alloc, description[1]);
  message_type m2(&alloc, description[1]);

  message_mref m1ref = m1.ref();
  message_mref m2ref = m2.ref();

  m1ref[0].as(1);

  nested_message_mref nested1(m1ref[1]);

  message_mref target1 = nested1.rebind(description[0]);
  target1[0].as(2);
  target1[1].as(3);

  nested_message_cref nested1_cref(m1.cref()[1]);

  REQUIRE(nested1_cref.target_instruction() ==  description[0]);
  const field_instruction* inst = nested1_cref.target().instruction();
  REQUIRE( dynamic_cast<const template_instruction*>(inst));

  message_cref the_nested1(nested1_cref.target());
  REQUIRE( static_cast<uint32_cref>(the_nested1[0]).value() ==  2U);
  REQUIRE( static_cast<uint32_cref>(the_nested1[1]).value() ==  3U);


  m2ref[0].as(1);

  nested_message_mref nested2(m2ref[1]);
  message_mref target2 = nested2.rebind(description[0]);
  target2[0].as(2);
  target2[1].as(3);

  REQUIRE(m1ref == m2ref);

  debug_allocator alloc2;

  message_type m3(m1.cref(), &alloc2);
  REQUIRE(m3.cref() == m1ref);
}


