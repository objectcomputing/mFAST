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
#include <mfast/coder/fast_encoder.h>
#include <mfast/vector_ref.h>
#include <mfast/coder/common/codec_helper.h>
#include <mfast/xml_parser/dynamic_templates_description.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>


typedef boost::mpl::list<int,long,unsigned char> test_types;
#include "debug_allocator.h"

using namespace mfast;

struct c_unique {
  int current;
  c_unique() {current=0;}
  int operator()() {return ++current;}
} ;


BOOST_AUTO_TEST_SUITE( test_int_vector )
//____________________________________________________________________________//

typedef boost::mpl::list<int32_t,uint32_t,int64_t, uint64_t> test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE( test_int_vector, T, test_types )
{
  debug_allocator alloc;
  value_storage storage;
  vector_field_instruction<T> inst(presence_optional,
                                   1, // id
                                   "int_vector","");

  inst.construct_value(storage, &alloc);
  BOOST_CHECK_EQUAL(storage.of_array.capacity_in_bytes_, 0UL);
  BOOST_CHECK_EQUAL(storage.is_defined(),                true);

  vector_mref<T> mref(&alloc, &storage, &inst);

  BOOST_CHECK_EQUAL(mref.size(), 0UL);

  const unsigned SIZE=100;
  int array[SIZE];
  std::generate(array, array+SIZE, c_unique());

  mref.assign(array, array+SIZE);

  BOOST_CHECK_EQUAL(mref.size(), SIZE);

  for (unsigned i = 0; i < SIZE; ++i) {
    BOOST_CHECK_EQUAL(mref[i], static_cast<T>(i+1));
  }

  inst.destruct_value(storage, &alloc);
}

BOOST_AUTO_TEST_CASE( test_int_vector_description)
{
  const char* xml_desc1 =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\">\n"
    "<uInt32Vector name=\"field1\" id=\"11\" />\n"
    "<int64Vector name=\"field2\" id=\"12\" />\n"
    "<uInt32 name=\"field3\" id=\"13\" />\n"
    "</template>\n"
    "</templates>\n";

  dynamic_templates_description desc1(xml_desc1);

  BOOST_CHECK_EQUAL(desc1.size(), 1UL);
  BOOST_CHECK_EQUAL(desc1[0]->subinstruction(0)->field_type(), field_type_uint32_vector);
  BOOST_CHECK_EQUAL(strcmp(desc1[0]->subinstruction(0)->name(), "field1"), 0);

  BOOST_CHECK_EQUAL(desc1[0]->subinstruction(1)->field_type(), field_type_int64_vector);
  BOOST_CHECK_EQUAL(strcmp(desc1[0]->subinstruction(1)->name(), "field2"), 0);
}

BOOST_AUTO_TEST_CASE (test_fast_encoding)
{
  const char* xml_desc1 =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\" id=\"1\">\n"
    "<uInt32Vector name=\"field1\" id=\"11\" />\n"
    "<int64Vector name=\"field2\" id=\"12\" />\n"
    "</template>\n"
    "</templates>\n";

  const char* xml_desc2 =
    "<?xml version=\" 1.0 \"?>\n"
    "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\" "
    "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
    "<template name=\"Test\" id=\"1\">\n"
    "<sequence name=\"field1\" id=\"11\">\n"
    "  <uInt32 name=\"content1\"/>"
    "</sequence>\n"
    "<sequence name=\"field2\" id=\"12\">\n"
    "  <int64 name=\"content2\"/>"
    "</sequence>\n"
    "</template>\n"
    "</templates>\n";

    debug_allocator alloc;

    dynamic_templates_description desc1(xml_desc1);

    const int SIZE= 6;
    int array [] = { 0, 1, 2, 3, 4, 5};

    message_type message1(&alloc, desc1[0]);
    std::vector<char> buffer1;

    {
      message_mref mref = message1.mref();
      uint32_vector_mref f0 = static_cast<uint32_vector_mref>(mref[0]);
      int64_vector_mref f1 = static_cast<int64_vector_mref>(mref[1]);

      f0.assign(array, array+SIZE);
      f1.assign(array, array+SIZE);

      const templates_description* descriptions[1] = { &desc1 };
      mfast::fast_encoder encoder;
      encoder.include(descriptions);
      buffer1.reserve(1024);
      encoder.encode(message1.cref(), buffer1);
    }


    dynamic_templates_description desc2(xml_desc2);
    message_type message2(&alloc, desc2[0]);
    std::vector<char> buffer2;

    {
      message_mref mref = message2.mref();
      sequence_mref f0 = static_cast<sequence_mref>(mref[0]);
      f0.resize(SIZE);
      for (int i = 0; i < SIZE; ++i) {
        f0[i][0].as<uint32_t>(i);
      }

      sequence_mref f1 = static_cast<sequence_mref>(mref[1]);
      f1.resize(SIZE);
      for (int64_t i = 0; i < SIZE; ++i) {
        f1[i][0].as<int64_t>(i);
      }

      const templates_description* descriptions[] = { &desc2 };
      mfast::fast_encoder encoder;
      encoder.include(descriptions);
      buffer2.reserve(1024);
      encoder.encode(message2.cref(), buffer2);
    }

    BOOST_CHECK_EQUAL(buffer1.size(), buffer2.size());

    BOOST_CHECK(std::equal(buffer1.begin(), buffer1.end(), buffer2.begin()));
}

BOOST_AUTO_TEST_SUITE_END()
