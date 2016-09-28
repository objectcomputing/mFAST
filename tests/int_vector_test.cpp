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
#include <mfast/coder/fast_encoder.h>
#include <mfast/vector_ref.h>
#include <mfast/coder/common/codec_helper.h>
#include <mfast/xml_parser/dynamic_templates_description.h>
#include <boost/mpl/list.hpp>


typedef boost::mpl::list<int,long,unsigned char> test_types;
#include "debug_allocator.h"

using namespace mfast;

struct c_unique {
  int current;
  c_unique() {current=0;}
  int operator()() {return ++current;}
} ;


//____________________________________________________________________________//


template <typename T>
void test_int_vector()
{
  debug_allocator alloc;
  value_storage storage;
  vector_field_instruction<T> inst(presence_optional,
                                   1, // id
                                   "int_vector","");

  inst.construct_value(storage, &alloc);
  REQUIRE((storage.of_array.capacity_in_bytes_ ==  0UL));
  REQUIRE((storage.is_defined() ==                 true));

  vector_mref<T> mref(&alloc, &storage, &inst);

  REQUIRE((mref.size() ==  0UL));

  const unsigned SIZE=100;
  int array[SIZE];
  std::generate(array, array+SIZE, c_unique());

  mref.assign(array, array+SIZE);

  REQUIRE((mref.size() ==  SIZE));

  for (unsigned i = 0; i < SIZE; ++i) {
    REQUIRE((mref[i] ==  static_cast<T>(i+1)));
  }

  inst.destruct_value(storage, &alloc);
}


TEST_CASE("test the fast encoding of int vectors", "[test_int_vector]")
{
  test_int_vector<int32_t>() ;
  test_int_vector<uint32_t>() ;
  test_int_vector<int64_t>() ;
  test_int_vector<uint64_t>() ;
}



TEST_CASE("test for template description with integer vectors","[test_int_vector_description]")
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

  REQUIRE(desc1.size() ==  1UL);
  REQUIRE(desc1[0]->subinstruction(0)->field_type() ==  field_type_uint32_vector);
  REQUIRE(strcmp(desc1[0]->subinstruction(0)->name(),  "field1") == 0);

  REQUIRE(desc1[0]->subinstruction(1)->field_type() ==  field_type_int64_vector);
  REQUIRE(strcmp(desc1[0]->subinstruction(1)->name(),  "field2") == 0);
}

TEST_CASE ("test fast encoding with integer vectors","[test_fast_encoding]")
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

    REQUIRE(buffer1.size() ==  buffer2.size());

    REQUIRE(std::equal(buffer1.begin(), buffer1.end(), buffer2.begin()));
}

