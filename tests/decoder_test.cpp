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

using namespace mfast;

bool str_equal(const char* lhs, const char* rhs)
{
  return std::strcmp(lhs, rhs) ==0;
}

BOOST_AUTO_TEST_SUITE( test_decoder )


BOOST_AUTO_TEST_CASE(simple_template_test)
{
  const char* xml_content =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy/></uInt32>\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy/></uInt32>\n"
    "</template>\n"
    "</templates>\n";

  dynamic_templates_description description(xml_content);
  decoder coder;

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);

  const char data[] = {
    "\xB8\x81\x82\x83"
  };

  const char* first = data;
  const char* last = data+sizeof(data)-1;

  message_cref msg = coder.decode(first, last);

  BOOST_CHECK_EQUAL(msg.fields_count(), 3);
  BOOST_CHECK_PREDICATE(str_equal, (msg.name())("Test"));

  field_cref f1 = msg.const_field(0);
  BOOST_CHECK_EQUAL(f1.id(),                        11);
  BOOST_CHECK_EQUAL(f1.field_type(), field_type_uint32);
  BOOST_CHECK_EQUAL(f1.optional(),               false);


  uint32_cref field1 = f1.static_cast_as<uint32_cref>();
  BOOST_CHECK_EQUAL(field1.value(),                  1);

  field_cref f2 = msg.const_field(1);
  BOOST_CHECK_EQUAL(f2.id(),                        12);
  BOOST_CHECK_EQUAL(f2.field_type(), field_type_uint32);
  BOOST_CHECK_EQUAL(f2.optional(),               false);

  uint32_cref field2 = f2.static_cast_as<uint32_cref>();
  BOOST_CHECK_EQUAL(field2.value(),                  2);

  field_cref f3 = msg.const_field(2);
  BOOST_CHECK_EQUAL(f3.id(),                        13);
  BOOST_CHECK_EQUAL(f3.field_type(), field_type_uint32);
  BOOST_CHECK_EQUAL(f3.optional(),               false);

  uint32_cref field3 = f3.static_cast_as<uint32_cref>();
  BOOST_CHECK_EQUAL(field3.value(),                  3);
}

BOOST_AUTO_TEST_CASE(group_test)
{
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
  decoder coder;

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);

  const char data[] = {
    "\xB0\x81\xE0\x82\x83"
  };

  const char* first = data;
  const char* last = data+sizeof(data)-1;

  message_cref msg = coder.decode(first, last);

  BOOST_CHECK_EQUAL(msg.fields_count(), 2);
  BOOST_CHECK_PREDICATE(str_equal, (msg.name())("Test"));

  field_cref f1 = msg.const_field(0);
  BOOST_CHECK_EQUAL(f1.id(),                        11);
  BOOST_CHECK_EQUAL(f1.field_type(), field_type_uint32);
  BOOST_CHECK_EQUAL(f1.optional(),               false);


  uint32_cref field1 = f1.static_cast_as<uint32_cref>();
  BOOST_CHECK_EQUAL(field1.value(),                 1);

  field_cref f2 = msg.const_field(1);
  BOOST_CHECK_EQUAL(f2.field_type(), field_type_group);
  BOOST_CHECK_EQUAL(f2.optional(),               true);

  group_cref field2 = f2.static_cast_as<group_cref>();
  BOOST_CHECK_EQUAL(field2.fields_count(),          2);
  {
    field_cref sub_f0 = field2.const_field(0);

    BOOST_CHECK_EQUAL(sub_f0.id(),                        12);
    BOOST_CHECK_EQUAL(sub_f0.field_type(), field_type_uint32);
    BOOST_CHECK_EQUAL(sub_f0.optional(),               false);

    uint32_cref sf0 = sub_f0.static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(sf0.value(),                         2);

    field_cref sub_f1 = field2.const_field(1);
    BOOST_CHECK_EQUAL(sub_f1.id(),                        13);
    BOOST_CHECK_EQUAL(sub_f1.field_type(), field_type_uint32);
    BOOST_CHECK_EQUAL(sub_f1.optional(),               false);

    uint32_cref sf1 = sub_f1.static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(sf1.value(),                         3);
  }
}


BOOST_AUTO_TEST_CASE(sequence_test)
{
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
  decoder coder;

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);

  const char data[] = {
    // pmap | f1 | s1 len| elem1 pmap | f2 | f3 | elem2 pmap |  f2 | f3 |
    //  A0    81    83        E0        82   83     E0          80   81
    "\xA0\x81\x83\xE0\x82\x83\xE0\x80\x81"
  };

  const char* first = data;
  const char* last = data+sizeof(data)-1;

  message_cref msg = coder.decode(first, last);

  BOOST_CHECK_EQUAL(msg.fields_count(), 2);
  BOOST_CHECK_PREDICATE(str_equal, (msg.name())("Test"));

  field_cref f1 = msg.const_field(0);
  BOOST_CHECK_EQUAL(f1.id(),                        11);
  BOOST_CHECK_EQUAL(f1.field_type(), field_type_uint32);
  BOOST_CHECK_EQUAL(f1.optional(),               false);


  uint32_cref field1 = f1.static_cast_as<uint32_cref>();
  BOOST_CHECK_EQUAL(field1.value(),                    1);

  field_cref f2 = msg.const_field(1);
  BOOST_CHECK_EQUAL(f2.field_type(), field_type_sequence);
  BOOST_CHECK_EQUAL(f2.optional(),                  true);

  sequence_cref seq_field = f2.static_cast_as<sequence_cref>();
  BOOST_CHECK_EQUAL(seq_field.size(),                  2);

  {
    sequence_element_cref elem0 = seq_field[0];

    BOOST_CHECK_EQUAL(elem0.fields_count(), 2);
    {
      field_cref sub_f0 = elem0.const_field(0);

      BOOST_CHECK_EQUAL(sub_f0.id(),                        12);
      BOOST_CHECK_EQUAL(sub_f0.field_type(), field_type_uint32);
      BOOST_CHECK_EQUAL(sub_f0.optional(),               false);

      uint32_cref sf0 = sub_f0.static_cast_as<uint32_cref>();
      BOOST_CHECK_EQUAL(sf0.value(),                         2);

      field_cref sub_f1 = elem0.const_field(1);
      BOOST_CHECK_EQUAL(sub_f1.id(),                        13);
      BOOST_CHECK_EQUAL(sub_f1.field_type(), field_type_uint32);
      BOOST_CHECK_EQUAL(sub_f1.optional(),               false);

      uint32_cref sf1 = sub_f1.static_cast_as<uint32_cref>();
      BOOST_CHECK_EQUAL(sf1.value(),                         3);
    }

    sequence_element_cref elem1 = seq_field[1];
    {
      field_cref sub_f0 = elem1.const_field(0);

      BOOST_CHECK_EQUAL(sub_f0.id(),                        12);
      BOOST_CHECK_EQUAL(sub_f0.field_type(), field_type_uint32);
      BOOST_CHECK_EQUAL(sub_f0.optional(),               false);

      uint32_cref sf0 = sub_f0.static_cast_as<uint32_cref>();
      BOOST_CHECK_EQUAL(sf0.value(),                         0);

      field_cref sub_f1 = elem1.const_field(1);
      BOOST_CHECK_EQUAL(sub_f1.id(),                        13);
      BOOST_CHECK_EQUAL(sub_f1.field_type(), field_type_uint32);
      BOOST_CHECK_EQUAL(sub_f1.optional(),               false);

      uint32_cref sf1 = sub_f1.static_cast_as<uint32_cref>();
      BOOST_CHECK_EQUAL(sf1.value(),                         1);
    }
  }
}

BOOST_AUTO_TEST_CASE(static_templateref_test)
{
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
  decoder coder;

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);

  const char data[] = {
    "\xF8\x82\x81\x82\x83"
  };

  const char* first = data;
  const char* last = data+sizeof(data)-1;

  message_cref msg = coder.decode(first, last);

  BOOST_CHECK_EQUAL(msg.fields_count(), 2);
  BOOST_CHECK_PREDICATE(str_equal, (msg.name())("Test"));

  field_cref f1 = msg.const_field(0);
  BOOST_CHECK_EQUAL(f1.id(),                        11);
  BOOST_CHECK_EQUAL(f1.field_type(), field_type_uint32);
  BOOST_CHECK_EQUAL(f1.optional(),               false);


  uint32_cref field1 = f1.static_cast_as<uint32_cref>();
  BOOST_CHECK_EQUAL(field1.value(),                    1);

  field_cref f2 = msg.const_field(1);
  BOOST_CHECK_EQUAL(f2.field_type(), field_type_template);

  message_cref field2 = f2.static_cast_as<message_cref>();
  BOOST_CHECK_EQUAL(field2.fields_count(),             2);
  {
    field_cref sub_f0 = field2.const_field(0);

    BOOST_CHECK_EQUAL(sub_f0.id(),                        12);
    BOOST_CHECK_EQUAL(sub_f0.field_type(), field_type_uint32);
    BOOST_CHECK_EQUAL(sub_f0.optional(),               false);

    uint32_cref sf0 = sub_f0.static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(sf0.value(),                         2);

    field_cref sub_f1 = field2.const_field(1);
    BOOST_CHECK_EQUAL(sub_f1.id(),                        13);
    BOOST_CHECK_EQUAL(sub_f1.field_type(), field_type_uint32);
    BOOST_CHECK_EQUAL(sub_f1.optional(),               false);

    uint32_cref sf1 = sub_f1.static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(sf1.value(),                         3);
  }
}

BOOST_AUTO_TEST_CASE(dynamic_templateref_test)
{
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
  decoder coder;

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);

  const char data[] = {
    "\xE0\x82\x81\xF0\x81\x82\x83"
  };

  const char* first = data;
  const char* last = data+sizeof(data)-1;

  message_cref msg = coder.decode(first, last);

  BOOST_CHECK_EQUAL(msg.fields_count(), 2);
  BOOST_CHECK_PREDICATE(str_equal, (msg.name())("Test"));

  field_cref f1 = msg.const_field(0);
  BOOST_CHECK_EQUAL(f1.id(),                        11);
  BOOST_CHECK_EQUAL(f1.field_type(), field_type_uint32);
  BOOST_CHECK_EQUAL(f1.optional(),               false);


  uint32_cref field1 = f1.static_cast_as<uint32_cref>();
  BOOST_CHECK_EQUAL(field1.value(),                       1);

  field_cref f2 = msg.const_field(1);
  BOOST_CHECK_EQUAL(f2.field_type(), field_type_templateref);

  message_cref field2 = f2.static_cast_as<dynamic_cref>().static_cast_as<message_cref>();
  BOOST_CHECK_EQUAL(field2.fields_count(),                2);
  {
    field_cref sub_f0 = field2.const_field(0);

    BOOST_CHECK_EQUAL(sub_f0.id(),                        12);
    BOOST_CHECK_EQUAL(sub_f0.field_type(), field_type_uint32);
    BOOST_CHECK_EQUAL(sub_f0.optional(),               false);

    uint32_cref sf0 = sub_f0.static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(sf0.value(),                         2);

    field_cref sub_f1 = field2.const_field(1);
    BOOST_CHECK_EQUAL(sub_f1.id(),                        13);
    BOOST_CHECK_EQUAL(sub_f1.field_type(), field_type_uint32);
    BOOST_CHECK_EQUAL(sub_f1.optional(),               false);

    uint32_cref sf1 = sub_f1.static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(sf1.value(),                         3);
  }
}

BOOST_AUTO_TEST_CASE(manual_reset_test)
{
  const char* xml_content =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy value=\"11\"/></uInt32>\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy value=\"12\"/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy value=\"13\"/></uInt32>\n"
    "</template>\n"
    "</templates>\n";

  dynamic_templates_description description(xml_content);
  decoder coder;

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);

  const char data[] = {
    "\xB8\x81\x82\x83"   // first message, |pmap|field1|field2|field3|
    "\x80"   // second message
    "\x80"   // third message
  };

  const char* first = data;
  const char* last = data+sizeof(data)-1;

  {
    // message_cref msg1 =
    coder.decode(first, last);
  }
  {
    message_cref msg2 = coder.decode(first, last);
    // msg2 should have the same value of meg1

    uint32_cref field1 = msg2.const_field(0).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field1.value(), 1);

    uint32_cref field2 = msg2.const_field(1).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field2.value(), 2);

    uint32_cref field3 = msg2.const_field(2).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field3.value(), 3);
  }
  {
    message_cref msg3 = coder.decode(first, last, true);
    // decoding with reset, all values should be defualts

    uint32_cref field1 = msg3.const_field(0).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field1.value(), 11);

    uint32_cref field2 = msg3.const_field(1).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field2.value(), 12);

    uint32_cref field3 = msg3.const_field(2).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field3.value(), 13);
  }
}

BOOST_AUTO_TEST_CASE(auto_reset_test)
{
  const char* xml_content =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\" scp:reset=\"yes\">\n"
    "<uInt32 name=\"field1\" id=\"11\"><copy value=\"11\"/></uInt32>\n"
    "<uInt32 name=\"field2\" id=\"12\"><copy value=\"12\"/></uInt32>\n"
    "<uInt32 name=\"field3\" id=\"13\"><copy value=\"13\"/></uInt32>\n"
    "</template>\n"
    "</templates>\n";

  dynamic_templates_description description(xml_content);
  decoder coder;

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);

  const char data[] = {
    "\xB8\x81\x82\x83"   // first message, |pmap|field1|field2|field3|
    "\x80"   // second message
    "\x80"   // third message
  };

  const char* first = data;
  const char* last = data+sizeof(data)-1;


  {
    // message_cref msg1 =
    coder.decode(first, last);
  }
  {
    message_cref msg2 = coder.decode(first, last, true);
    // decoding with reset, all values should be defualts

    uint32_cref field1 = msg2.const_field(0).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field1.value(), 11);

    uint32_cref field2 = msg2.const_field(1).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field2.value(), 12);

    uint32_cref field3 = msg2.const_field(2).static_cast_as<uint32_cref>();
    BOOST_CHECK_EQUAL(field3.value(), 13);
  }
}


class mock_allocator
  :   public allocator
{
  const bool resettable_;
  intptr_t buffer_[1024];
  intptr_t* gptr_;
  static const std::size_t alignment = sizeof(intptr_t);

  public:
    uint32_t reset_called_count_;
    uint32_t allocate_called_count_;
    uint32_t deallocate_called_count_;
    uint32_t reallocate_called_count_;


    mock_allocator(bool resettable)
      : resettable_(resettable)
      , gptr_(buffer_)
      , reset_called_count_(0)
      , allocate_called_count_(0)
      , deallocate_called_count_(0)
      , reallocate_called_count_(0)
    {
    }

    virtual void* allocate(std::size_t n)
    {
      ++allocate_called_count_;

      std::size_t alloc_size = n/sizeof(intptr_t) + n%sizeof(intptr_t);
      void* result = gptr_;
      gptr_ += alloc_size;
      return result;
    }

    virtual std::size_t reallocate(void*& pointer, std::size_t /* old_size */, std::size_t new_size)
    {
      std::size_t alloc_size = new_size/sizeof(intptr_t) + new_size%sizeof(intptr_t);
      pointer = gptr_;
      gptr_ += alloc_size;

      ++reallocate_called_count_;
      return alloc_size*sizeof(intptr_t);
    }

    virtual void deallocate(void* /* pointer */)
    {
      ++deallocate_called_count_;
    }

    virtual bool reset()
    {
      gptr_ = buffer_;
      ++reset_called_count_;
      return resettable_;
    }

};

bool str_equal(const char* lhs, const char* rhs)
{
  return std::strcmp(lhs, rhs) ==0;
}

BOOST_AUTO_TEST_CASE(allocator_decode_test)
{
  const char* xml_content =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\" scp:reset=\"yes\">\n"
    "<string name=\"field1\" id=\"11\"><copy/></string>\n"
    "<sequence name=\"sequence1\" presence=\"optional\">"
    "<string name=\"field2\" id=\"12\"><copy/></string>\n"
    "<string name=\"field3\" id=\"13\"><copy/></string>\n"
    "</sequence>"
    "</template>\n"
    "</templates>\n";

  const char data[] = {
    // |pmap|field1|sequence len|elem1 pmap|field2|field3|
    //   A0   C1(A)      82           E0     C2(B)  C3(C)
    "\xA0\xC1\x82\xE0\xC2\xC3"
    // |pmap|field1|sequence len|elem1 pmap|field2|field3|elem2 pmap|field2|field3|
    //   A0   D1(Q)      83           E0     D2(R)  D3(S)     E0      D0(P)   D1(Q)
    "\xA0\xD1\x83\xE0\xD2\xD3\xE0\xD0\xD1"
  };

  dynamic_templates_description description(xml_content);

  mock_allocator alloc(true);
  decoder coder(&alloc);

  const templates_description* descriptions[] = { &description };

  coder.include(descriptions);
  BOOST_CHECK_EQUAL(alloc.allocate_called_count_, 1);

  const char* first = data;
  const char* last = data+sizeof(data)-1;

  {
    message_cref msg1 = coder.decode(first, last);
    BOOST_CHECK_EQUAL(alloc.allocate_called_count_,   2);
    BOOST_CHECK_EQUAL(alloc.reallocate_called_count_, 4);
    BOOST_CHECK_EQUAL(alloc.deallocate_called_count_, 0);
    BOOST_CHECK_EQUAL(alloc.reset_called_count_,      1);

    ascii_string_cref field1 = msg1.const_field(0).static_cast_as<ascii_string_cref>();
    BOOST_CHECK_PREDICATE(str_equal, (field1.c_str())("A"));

    sequence_cref field2 = msg1.const_field(1).static_cast_as<sequence_cref>();
    BOOST_CHECK_EQUAL(field2.size(), 1);

    BOOST_CHECK_PREDICATE(str_equal, (field2[0].const_field(0).static_cast_as<ascii_string_cref>().c_str())("B"));
    BOOST_CHECK_PREDICATE(str_equal, (field2[0].const_field(1).static_cast_as<ascii_string_cref>().c_str())("C"));
  }

  {
    message_cref msg2 = coder.decode(first, last);
    BOOST_CHECK_EQUAL(alloc.allocate_called_count_,    3);
    BOOST_CHECK_EQUAL(alloc.reallocate_called_count_, 10);
    BOOST_CHECK_EQUAL(alloc.deallocate_called_count_,  0);
    BOOST_CHECK_EQUAL(alloc.reset_called_count_,       2);

    ascii_string_cref field1 = msg2.const_field(0).static_cast_as<ascii_string_cref>();
    BOOST_CHECK_PREDICATE(str_equal, (field1.c_str())("Q"));

    sequence_cref field2 = msg2.const_field(1).static_cast_as<sequence_cref>();
    BOOST_CHECK_EQUAL(field2.size(), 2);

    BOOST_CHECK_PREDICATE(str_equal, (field2[0].const_field(0).static_cast_as<ascii_string_cref>().c_str())("R"));
    BOOST_CHECK_PREDICATE(str_equal, (field2[0].const_field(1).static_cast_as<ascii_string_cref>().c_str())("S"));

    BOOST_CHECK_PREDICATE(str_equal, (field2[1].const_field(0).static_cast_as<ascii_string_cref>().c_str())("P"));
    BOOST_CHECK_PREDICATE(str_equal, (field2[1].const_field(1).static_cast_as<ascii_string_cref>().c_str())("Q"));
  }
}


BOOST_AUTO_TEST_SUITE_END()
