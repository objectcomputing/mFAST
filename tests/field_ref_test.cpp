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



#include <mfast/int_ref.h>
#include <mfast/string_ref.h>
#include <mfast/decimal_ref.h>
#include <mfast/group_ref.h>
#include <mfast/sequence_ref.h>
#include <mfast/coder/common/codec_helper.h>

#define BOOST_TEST_DYN_LINK
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "debug_allocator.h"

using namespace mfast;

BOOST_AUTO_TEST_SUITE( test_field_ref )

BOOST_AUTO_TEST_CASE(integer_field_test)
{
  malloc_allocator allocator;
  value_storage storage;

  uint64_cref null_ref;

  BOOST_CHECK(!null_ref.present() );

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  0,
                                  int_value_storage<uint64_t>(UINT64_MAX));

    BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), false);

    inst.construct_value(storage, &allocator);

    uint64_cref cref(&storage, &inst);

    BOOST_CHECK(!cref.present() );
    BOOST_CHECK_EQUAL(cref.id(), 1U);
    BOOST_CHECK( strcmp(cref.name(), "test_uint64") == 0);

    uint64_mref ref(&allocator, &storage, &inst);

    BOOST_CHECK(!ref.present() );
    BOOST_CHECK_EQUAL(ref.id(), 1U);
    BOOST_CHECK( strcmp(ref.name(), "test_uint64") == 0);

    ref.as(5);
    BOOST_CHECK( ref.present() );
    BOOST_CHECK_EQUAL(ref.value(), 5U);

    ref.to_initial_value();
    BOOST_CHECK( ref.present() );
    BOOST_CHECK_EQUAL(ref.value(), UINT64_MAX);

    // test convertion from field_mref to field_cref

    uint64_cref another_cref(ref);
    BOOST_CHECK_EQUAL(another_cref.value(), UINT64_MAX);


    ref.omit();
    BOOST_CHECK(!ref.present() );

    detail::codec_helper helper;

    BOOST_CHECK(!helper.previous_value_of(ref).is_defined() );

    // The base value depends on the state of the previous value in the following way:
    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    // * empty – it is a dynamic error [ERR D6] if the previous value is empty.


    {
      value_storage base_value = helper.delta_base_value_of(ref);
      uint64_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK_EQUAL(base_cref.value(), UINT64_MAX);
    }
    {
      helper.save_previous_value(ref);
      BOOST_CHECK_THROW(helper.delta_base_value_of(ref), mfast::fast_error);
    }
    {
      ref.as(4);
      helper.save_previous_value(ref);
      value_storage base_value = helper.delta_base_value_of(ref);
      uint64_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK_EQUAL(base_cref.value(), 4U);
    }
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  0,
                                  int_value_storage<uint64_t>(UINT64_MAX));


    inst.construct_value(storage, &allocator);

    uint64_cref cref(&storage, &inst);

    BOOST_CHECK(cref.present() );
    BOOST_CHECK_EQUAL(cref.id(), 1U);
    BOOST_CHECK( strcmp(cref.name(), "test_uint64") == 0);
    BOOST_CHECK_EQUAL( cref.value(), UINT64_MAX);

    uint64_mref ref(&allocator, &storage, &inst);
    BOOST_CHECK(ref.present() );
    ref.omit(); // mandatory field shoudn't be able to be absent
    BOOST_CHECK(ref.present() );




  }

  {
    int32_field_instruction inst(operator_copy,
                                 presence_mandatory,
                                 1,
                                 "test_int32","",
                                 0,
                                 int_value_storage<int32_t>(INT32_MIN));

    inst.construct_value(storage, &allocator);

    int32_cref cref(&storage, &inst);
    BOOST_CHECK(cref.present() );
    BOOST_CHECK_EQUAL(cref.value(), INT32_MIN);
  }


  uint32_field_instruction inst(operator_copy,
                                presence_mandatory,
                                1,
                                "test_uint32","",
                                0,
                                int_value_storage<uint32_t>());

  BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), true);

}

BOOST_AUTO_TEST_CASE(decimal_field_instruction_test)
{
  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0,
                                   decimal_value_storage(INT64_MAX,64));
    BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), false);
  }

  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0,
                                   decimal_value_storage());
    BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), true);
  }
  {
    mantissa_field_instruction mantissa_inst(operator_copy,
                                             0,
                                             int_value_storage<int64_t>());

    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0,
                                   &mantissa_inst,
                                   decimal_value_storage());
    BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), true);
  }
  {
    mantissa_field_instruction mantissa_inst(operator_copy,
                                             0,
                                             int_value_storage<int64_t>());

    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0,
                                   &mantissa_inst,
                                   decimal_value_storage(0, 3));
    BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), false);
  }
}

BOOST_AUTO_TEST_CASE(decimal_field_test)
{
  malloc_allocator allocator;
  value_storage storage;

  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   0,
                                   decimal_value_storage(INT64_MAX,64));

    inst.construct_value(storage, &allocator);
    BOOST_CHECK(inst.mantissa_instruction() == 0);

    decimal_cref cref(&storage, &inst);

    BOOST_CHECK(!cref.present() );
    BOOST_CHECK_EQUAL(cref.id(), 1U);
    BOOST_CHECK( strcmp(cref.name(), "test_decimal") == 0);

    decimal_mref ref(&allocator, &storage, &inst);

    BOOST_CHECK(!ref.present() );
    BOOST_CHECK_EQUAL(ref.id(), 1U);
    BOOST_CHECK( strcmp(ref.name(), "test_decimal") == 0);

    ref.as(5, 20);
    BOOST_CHECK( ref.present() );
    BOOST_CHECK_EQUAL(ref.mantissa(), 5);
    BOOST_CHECK_EQUAL(ref.exponent(), 20);

    ref.to_initial_value();
    BOOST_CHECK( ref.present() );
    BOOST_CHECK_EQUAL(ref.mantissa(), INT64_MAX);
    BOOST_CHECK_EQUAL(ref.exponent(), 64);

    ref.omit();
    BOOST_CHECK(!ref.present() );

    ref.set_mantissa(4);
    BOOST_CHECK(ref.present() );
    BOOST_CHECK_EQUAL(ref.mantissa(), 4);
    BOOST_CHECK_EQUAL(ref.exponent(), 64);

    ref.omit();
    ref.set_exponent(5);
    BOOST_CHECK(ref.present() );
    BOOST_CHECK_EQUAL(ref.exponent(), 5);


    // test convertion from field_mref to field_cref
    decimal_cref another_cref(ref);
    BOOST_CHECK_EQUAL(another_cref.mantissa(), 4);
    BOOST_CHECK_EQUAL(another_cref.exponent(), 5);

    // test delta base value

    detail::codec_helper helper;

    BOOST_CHECK(!helper.previous_value_of(ref).is_defined() );

    // The base value depends on the state of the previous value in the following way:
    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    // * empty – it is a dynamic error [ERR D6] if the previous value is empty.

    {
      value_storage base_value = helper.delta_base_value_of(ref);
      decimal_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK_EQUAL(base_cref.mantissa(), INT64_MAX);
      BOOST_CHECK_EQUAL(base_cref.exponent(), 64);

    }
    {
      ref.omit();
      helper.save_previous_value(ref);
      BOOST_CHECK_THROW(helper.delta_base_value_of(ref), mfast::fast_error);
    }
    {
      ref.set_mantissa(4);
      ref.set_exponent(0);
      helper.save_previous_value(ref);
      value_storage base_value = helper.delta_base_value_of(ref);
      decimal_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK_EQUAL(base_cref.mantissa(), 4);
      BOOST_CHECK_EQUAL(base_cref.exponent(), 0);

    }

    // test conversion from decimal value

    ref.as(decimal(500));
    BOOST_CHECK_EQUAL(ref.mantissa(), 5);
    BOOST_CHECK_EQUAL(ref.exponent(), 2);

    ref.as(decimal("0.05"));
    BOOST_CHECK_EQUAL(ref.mantissa(), 5);
    BOOST_CHECK_EQUAL(ref.exponent(), -2);

    ref.as(-110);
    BOOST_CHECK_EQUAL(ref.value(), decimal(-110));

    ref.as("123.45");
    BOOST_CHECK_EQUAL(ref.mantissa(), 12345);
    BOOST_CHECK_EQUAL(ref.exponent(), -2);
  }
}

BOOST_AUTO_TEST_CASE(decimal_field_test2)
{
  malloc_allocator allocator;
  value_storage storage;

  {
    mantissa_field_instruction mantissa_inst(operator_copy, 0, int_value_storage<int64_t>(6));
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal2","",
                                   0,
                                   &mantissa_inst,
                                   decimal_value_storage(0,64));

    inst.construct_value(storage, &allocator);

    decimal_cref cref(&storage, &inst);

    BOOST_CHECK(!cref.present() );
    BOOST_CHECK_EQUAL(cref.id(), 1U);
    BOOST_CHECK( strcmp(cref.name(), "test_decimal2") == 0);

    decimal_mref ref(&allocator, &storage, &inst);

    BOOST_CHECK(!ref.present() );
    BOOST_CHECK_EQUAL(ref.id(), 1U);
    BOOST_CHECK( strcmp(ref.name(), "test_decimal2") == 0);

    ref.as(5, 20);
    BOOST_CHECK( ref.present() );
    BOOST_CHECK_EQUAL(ref.mantissa(), 5);
    BOOST_CHECK_EQUAL(ref.exponent(), 20);

    ref.to_initial_value();
    BOOST_CHECK( ref.present() );
    BOOST_CHECK_EQUAL(ref.mantissa(), 6);
    BOOST_CHECK_EQUAL(ref.exponent(), 64);

    ref.omit();
    BOOST_CHECK(!ref.present() );

    ref.set_mantissa(4);
    BOOST_CHECK(ref.present() );
    BOOST_CHECK_EQUAL(ref.mantissa(), 4);
    BOOST_CHECK_EQUAL(ref.exponent(), 64);

    ref.omit();
    ref.set_exponent(5);
    BOOST_CHECK(ref.present() );
    BOOST_CHECK_EQUAL(ref.exponent(), 5);

    exponent_mref exp_mref = ref.set_exponent();
    exp_mref.to_initial_value();
    BOOST_CHECK_EQUAL(exp_mref.value(), 64);
    BOOST_CHECK_EQUAL(ref.exponent(),   64);

  }

  {
    mantissa_field_instruction mantissa_inst(operator_copy, 0, int_value_storage<int64_t>(6));
    decimal_field_instruction inst(operator_default,
                                   presence_optional,
                                   1,
                                   "test_decimal2","",
                                   0,
                                   &mantissa_inst,
                                   decimal_value_storage());

    inst.construct_value(storage, &allocator);

    decimal_mref ref(&allocator, &storage, &inst);
    ref.as(1,1); // set the value so it's present

    // The exponent initial value is absent, therefore, the entire value should be absent().
    ref.set_exponent().to_initial_value();
    BOOST_CHECK(ref.absent());
  }

  {
    mantissa_field_instruction mantissa_inst(operator_copy, 0, int_value_storage<int64_t>(6));
    decimal_field_instruction inst(operator_constant,
                                   presence_mandatory,
                                   1,
                                   "test_decimal3","",
                                   0,
                                   &mantissa_inst,
                                   decimal_value_storage(0, -1));

    inst.construct_value(storage, &allocator);

    decimal_mref ref(&allocator, &storage, &inst);
    ref.as(1,1); // set the value so it's present

    ref.normalize();
    BOOST_CHECK_EQUAL(ref.mantissa(), 100);
    BOOST_CHECK_EQUAL(ref.exponent(), -1);

    ref.as(12345, -3);
    ref.normalize();
    BOOST_CHECK_EQUAL(ref.mantissa(), 123);
    BOOST_CHECK_EQUAL(ref.exponent(), -1);

  }
  {
    mfast::decimal_type x;
    x.mref ().as (9664.3);

    double y(x.cref());
    BOOST_CHECK_CLOSE(y, 9664.3, 0.00001);

    x.mref ().as (0);
    BOOST_CHECK_EQUAL(x.cref().mantissa(), 0LL);
  }

}

BOOST_AUTO_TEST_CASE(string_field_instruction_test)
{
  debug_allocator alloc;
  value_storage storage;
  const char* default_value = "initial_string";
  {
    ascii_field_instruction inst(operator_copy,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 string_value_storage(default_value));

    BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), false);
  }
  {
    ascii_field_instruction inst(operator_copy,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 0,
                                 string_value_storage());

    BOOST_CHECK_EQUAL(inst.initial_value().is_empty(), true);
  }
}

BOOST_AUTO_TEST_CASE(string_field_test)
{
  debug_allocator alloc;
  value_storage storage;
  const char* default_value = "initial_string";
  ascii_field_instruction inst(operator_copy,
                               presence_optional,
                               1,
                               "test_ascii","",
                               0,
                               string_value_storage(default_value));

  inst.construct_value(storage, &alloc);
  BOOST_CHECK_EQUAL(inst.initial_value().is_empty(),     false);

  BOOST_CHECK_EQUAL(storage.of_array.capacity_in_bytes_, 0U);

  {
    ascii_string_cref cref(&storage, &inst);
    BOOST_CHECK(!cref.present() );
    BOOST_CHECK_EQUAL(cref.id(), 1U);
    BOOST_CHECK( strcmp(cref.name(), "test_ascii") == 0);
  }

  {
    ascii_string_mref ref(&alloc, &storage, &inst);
    BOOST_CHECK(!ref.present());

    ref.push_back('c');
    BOOST_CHECK_EQUAL(ref.size(), 1U);

    ref.to_initial_value();
    BOOST_CHECK(ref.present());
    BOOST_CHECK_EQUAL(ref.size(), strlen(default_value));
    BOOST_CHECK(ref == "initial_string" );

    // testing value
    std::stringstream strm;
    strm << ref.value();
    BOOST_CHECK_EQUAL(strm.str(), std::string("initial_string"));

    ref.as("string1");
    BOOST_CHECK(ref == "string1" );

    ref.as("string2");
    BOOST_CHECK(ref == "string2" );

    ref += "abc";
    BOOST_CHECK(ref == "string2abc" );

    ref.resize(1);
    BOOST_CHECK(ref == "s");

    ref.replace(0, 0, "abcd", 4);
    BOOST_CHECK(ref == "abcds");

    ref.replace(0, 2, "xxxx", 4);
    BOOST_CHECK(ref == "xxxxcds");

    ref.replace(4, 3, "yy", 2);
    BOOST_CHECK(ref == "xxxxyy");

    ref.replace(4, 2, "aaaa", 4);
    BOOST_CHECK(ref == "xxxxaaaa");

    ref.refers_to("abcde");
    BOOST_CHECK(ref == "abcde");
    BOOST_CHECK_EQUAL(storage.of_array.capacity_in_bytes_, 0U);

    ref.pop_back();
    BOOST_CHECK(ref == "abcd");

    ref.clear();
    BOOST_CHECK(ref.absent());
    ref.as("cde");
    BOOST_CHECK(ref == "cde");

    ref.omit();
    BOOST_CHECK(ref.absent());



    detail::codec_helper helper;

    BOOST_CHECK(!helper.previous_value_of(ref).is_defined() );


    // testing delta base value

    // The base value depends on the state of the previous value in the following way:
    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    // * empty – it is a dynamic error [ERR D6] if the previous value is empty.

    {
      value_storage base_value = helper.delta_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK( base_cref == "initial_string" );
    }
    {
      helper.save_previous_value(ref);
      BOOST_CHECK_THROW(helper.delta_base_value_of(ref), mfast::fast_error);
    }
    {
      ref.as("4");
      helper.save_previous_value(ref);
      value_storage base_value = helper.delta_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK( base_cref == "4");
    }

    // testing tail base value

    // The base value depends on the state of the previous value in the following way:
    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    // * empty – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    //
    {
      value_storage base_value = helper.tail_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK( base_cref == "4");
    }
    ref.omit();
    helper.save_previous_value(ref);
    {
      value_storage base_value = helper.tail_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK( base_cref == "initial_string" );
    }

    helper.previous_value_of(ref).defined(false);

    {
      value_storage base_value = helper.tail_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      BOOST_CHECK( base_cref.present() );
      BOOST_CHECK( base_cref == "initial_string" );
    }

  }


  {
    // testing swap operation

    ascii_string_type string_holder1(&alloc);
    ascii_string_type string_holder2(&alloc);

    string_holder1.mref().as("abc");

    BOOST_CHECK_EQUAL(string_holder1.cref().size(), 3U);
    BOOST_CHECK_EQUAL(boost::string_ref("abc"), string_holder1.cref().value());


    string_holder2.mref().as("def");
    string_holder1.mref().swap(string_holder2.mref());

    BOOST_CHECK_EQUAL(boost::string_ref("def"), string_holder1.cref().value());
    BOOST_CHECK_EQUAL(boost::string_ref("abc"), string_holder2.cref().value());
  }

  inst.destruct_value(storage, &alloc);

}



BOOST_AUTO_TEST_CASE(string_delta_test)
{
  debug_allocator alloc;
  value_storage storage;
  const char* default_value = "initial_string";
  ascii_field_instruction inst(operator_copy,
                               presence_optional,
                               1,
                               "test_ascii","",
                               0,
                               string_value_storage(default_value));

  inst.construct_value(storage, &alloc);
  detail::codec_helper helper;

  {
    ascii_string_mref mref(&alloc, &storage, &inst);
    const char* base_str = "base_value";
    value_storage base_value;
    base_value.of_array.content_ = const_cast<char*>(base_str);
    base_value.array_length( static_cast<uint32_t>(strlen(base_str)) );


    const char* delta_str = "\x41\x42\x43\x44\xC5"; // "ABCDE"
    uint32_t delta_len = static_cast<uint32_t>(strlen(delta_str));

    helper.apply_string_delta(mref, base_value, 5, delta_str, delta_len);

    const char* result = "base_ABCDE";
    BOOST_CHECK_EQUAL_COLLECTIONS( result, result+10, mref.data(), mref.data() + mref.size());


    helper.apply_string_delta(mref, base_value, -5, delta_str, delta_len);
    result = "ABCDE_value";
    BOOST_CHECK_EQUAL_COLLECTIONS( result, result+11, mref.data(), mref.data() + mref.size());
  }

  inst.destruct_value(storage, &alloc);

}


BOOST_AUTO_TEST_CASE(group_field_test)
{
  debug_allocator alloc;
  value_storage storage;

  unsigned char f0_initial[] = "\x01\x02\x03\x04\x05";
  const char* f1_initial = "abcdefg";

  byte_vector_field_instruction inst0(operator_copy,
                                      presence_optional,
                                      1,
                                      "test_byte_vector","",
                                      0,
                                      byte_vector_value_storage(f0_initial,sizeof(f0_initial)),
                                      0, 0, 0);

  unicode_field_instruction inst1(operator_copy,
                                  presence_optional,
                                  2,
                                  "test_unicode","",
                                  0,
                                  string_value_storage(f1_initial), 0, "", "");

  const field_instruction* instructions[] = {
    &inst0,&inst1
  };

  group_field_instruction group_inst(presence_optional,
                                     3,
                                     "test_group","","",
                                     instructions,
                                     "", "", "");

  BOOST_CHECK_EQUAL(group_inst.group_content_byte_count(), 2 * sizeof(value_storage) );

  group_inst.construct_value(storage, &alloc);

  BOOST_CHECK(storage.of_group.content_ != 0);
  {
    group_cref ref(&storage, &group_inst);
    BOOST_CHECK_EQUAL(ref.present(),                         false);
    BOOST_CHECK_EQUAL(ref.num_fields(),                      2U);

    BOOST_CHECK_EQUAL(aggregate_cref(ref).subinstruction(0), &inst0);
    BOOST_CHECK_EQUAL(aggregate_cref(ref).subinstruction(1), &inst1);

    field_cref f0( ref[0] );
    BOOST_CHECK(f0.absent());

    field_cref f1( ref[1] );
    BOOST_CHECK(f1.absent());


    byte_vector_cref cf0(f0);
    BOOST_CHECK_EQUAL(cf0.field_type(), field_type_byte_vector);

    unicode_string_cref cf1(f1);
    BOOST_CHECK_EQUAL(cf1.field_type(), field_type_unicode_string);
  }

  {
    group_mref ref(&alloc, &storage, &group_inst);
    BOOST_CHECK_EQUAL(ref.present(),    true);
    BOOST_CHECK_EQUAL(ref.num_fields(), 2U);

    field_mref f0(ref[0] );
    BOOST_CHECK(f0.absent());

    BOOST_CHECK(ref.present());

    field_mref f1( ref[1] );
    BOOST_CHECK(f1.absent());


    byte_vector_mref mf0(f0);
    BOOST_CHECK_EQUAL(mf0.field_type(), field_type_byte_vector);

    unicode_string_mref mf1(f1);
    BOOST_CHECK_EQUAL(mf1.field_type(), field_type_unicode_string);

    mf0.to_initial_value();

    BOOST_CHECK_EQUAL(mf0.size(),                                 sizeof(f0_initial));
    BOOST_CHECK_EQUAL(memcmp(mf0.data(), f0_initial, mf0.size()), 0);

    unsigned char bytes[] = "test";
    mf0.assign(bytes, bytes+4);

    BOOST_CHECK_EQUAL(mf0.size(),                            4U);
    BOOST_CHECK_EQUAL(memcmp(mf0.data(), bytes, mf0.size()), 0);

    mf0.refers_to(bytes, 4);
    BOOST_CHECK_EQUAL(mf0.data(), bytes);
    BOOST_CHECK_EQUAL(mf0.size(), 4U);
  }

  group_inst.destruct_value(storage, &alloc);

}

BOOST_AUTO_TEST_CASE(sequence_field_test)
{
  debug_allocator alloc;
  value_storage storage;

  unsigned char f0_initial[] = "\x01\x02\x03\x04\x05";
  const char* f1_initial = "abcdefg";

  byte_vector_field_instruction inst0(operator_copy,
                                      presence_optional,
                                      1,
                                      "test_byte_vector","",
                                      0,
                                      byte_vector_value_storage(f0_initial,sizeof(f0_initial)),
                                      0, 0, 0);

  unicode_field_instruction inst1(operator_copy,
                                  presence_optional,
                                  2,
                                  "test_unicode","",
                                  0,
                                  string_value_storage(f1_initial), 0, "", "");

  uint32_field_instruction length_inst(operator_none,
                                       presence_mandatory,
                                       4,
                                       "","",
                                       0,
                                       2);


  const field_instruction* instructions[] = {
    &inst0,&inst1
  };

  sequence_field_instruction sequence_inst(presence_optional,
                                           3, // id
                                           "test_group","","",
                                           instructions,0,0,
                                           &length_inst, "", "", "");

  BOOST_CHECK_EQUAL(sequence_inst.subinstructions().size(),    2U);
  BOOST_CHECK_EQUAL(sequence_inst.group_content_byte_count(), 2 * sizeof(value_storage) );

  sequence_inst.construct_value(storage, &alloc);

  {
    sequence_mref ref(&alloc, &storage, &sequence_inst);
    BOOST_CHECK_EQUAL(ref.present(),    false);
    BOOST_CHECK_EQUAL(ref.num_fields(), 2U);

    ref.resize(2);
    BOOST_CHECK_EQUAL(ref.present(),    true);

    ref.omit();
    BOOST_CHECK_EQUAL(ref.present(),    false);

    ref.resize(2);
    BOOST_CHECK_EQUAL(ref.present(),    true);

    sequence_element_mref e0ref(ref[0]);
    sequence_element_cref e0cref(ref[0]);


    field_mref e0f0(e0ref[0] );
    BOOST_CHECK(e0f0.absent());

    field_mref e0f1( e0ref[1] );
    BOOST_CHECK(e0f1.absent());

    {
      byte_vector_mref mf0(e0f0);
      BOOST_CHECK_EQUAL(mf0.field_type(), field_type_byte_vector);

      unicode_string_mref mf1(e0f1);
      BOOST_CHECK_EQUAL(mf1.field_type(), field_type_unicode_string);

      mf0.to_initial_value();

      BOOST_CHECK_EQUAL(mf0.size(),                                 sizeof(f0_initial));
      BOOST_CHECK_EQUAL(memcmp(mf0.data(), f0_initial, mf0.size()), 0);

      unsigned char bytes[] = "test";
      mf0.assign(bytes, bytes+4);

      BOOST_CHECK_EQUAL(mf0.size(),                            4U);
      BOOST_CHECK_EQUAL(memcmp(mf0.data(), bytes, mf0.size()), 0);
    }

    ref.resize(4);
    BOOST_CHECK_EQUAL(ref.size(), 4U);
    {
      field_mref e3f1(ref[3][1] );
      BOOST_CHECK(e3f1.absent());

      field_cref const_e3f1(ref[3][1] );
      BOOST_CHECK(const_e3f1.absent());

      unicode_string_mref str(e3f1);
      BOOST_CHECK_EQUAL(str.field_type(), field_type_unicode_string);

      str.as( "abcdef" );
      BOOST_CHECK(const_e3f1.present());

      unicode_string_cref cstr(e3f1);
      BOOST_CHECK(cstr == str);
    }
  }

  sequence_inst.destruct_value(storage, &alloc);

}


class mock_field_instruction
  : public field_instruction
{
public:


  mock_field_instruction()
    : field_instruction (operator_none,field_type_int32,presence_mandatory,0,"","", instruction_tag())
    , construct_value_called_(0)
    , destruct_value_called_(0)
    , copy_value_deep_called_(0)
  {
  }

  mutable int construct_value_called_;
  mutable int destruct_value_called_;
  mutable int copy_value_deep_called_;


  virtual void construct_value(value_storage& /* storage */,
                               allocator*       /* alloc */) const
  {
    construct_value_called_++;
  }

  virtual void destruct_value(value_storage& /* storage */,
                              allocator*       /* alloc */) const
  {
    destruct_value_called_++;
  }

  /// Perform deep copy
  virtual void copy_value(const value_storage& /* src */,
                          value_storage      & /* dest */,
                          allocator*             /* alloc */) const
  {
    copy_value_deep_called_++;
  }

  virtual void accept(field_instruction_visitor&, void*) const
  {
  }

  virtual mock_field_instruction* clone(arena_allocator& alloc) const
  {
    return new (alloc) mock_field_instruction(*this);
  }
};


class simple_debug_allocator
  : public debug_allocator
{
public:

  simple_debug_allocator()
  {
  }

  virtual std::size_t reallocate(void*& pointer, std::size_t old_size, std::size_t new_size)
  {
    void* old_ptr = pointer;
    if (pointer) {
      pointer = std::realloc(pointer, new_size);
      std::memset(static_cast<char*>(pointer)+old_size, 0xFF, new_size-old_size);
    }
    else
      pointer = std::malloc(new_size);
    leased_addresses_.erase(old_ptr);
    if (pointer == 0) {
      std::free(old_ptr);
      throw std::bad_alloc();
    }
    leased_addresses_.insert(pointer);
    return new_size;
  }

};


BOOST_AUTO_TEST_CASE(sequence_resize_test)
{
  simple_debug_allocator alloc;
  value_storage storage;

  mock_field_instruction mock0;

  const field_instruction* instructions[] = {
    &mock0
  };

  uint32_field_instruction length_inst(operator_none,
                                       presence_mandatory,
                                       4,
                                       "","",
                                       0,
                                       int_value_storage<uint32_t>(0));

  sequence_field_instruction sequence_inst(presence_optional,
                                           3, // id
                                           "test_group","","",
                                           instructions,0,0,
                                           &length_inst, "", "", "");

  sequence_inst.construct_value(storage, &alloc);


  {
    sequence_mref ref(&alloc, &storage, &sequence_inst);
    BOOST_CHECK_EQUAL(ref.present(),                 false);
    BOOST_CHECK_EQUAL(ref.num_fields(),              1U);

    ref.reserve(2);
    BOOST_CHECK_EQUAL(ref.size(),                    0U);
    BOOST_CHECK_EQUAL(mock0.construct_value_called_, 2);
    BOOST_CHECK_EQUAL(mock0.destruct_value_called_,  0);


    ref.resize(2);
    BOOST_CHECK_EQUAL(ref.present(),                 true);
    BOOST_CHECK_EQUAL(ref.size(),                    2U);

    BOOST_CHECK_EQUAL(mock0.construct_value_called_, 2);
    BOOST_CHECK_EQUAL(mock0.destruct_value_called_,  0);


    ref.resize(1);
    BOOST_CHECK_EQUAL(mock0.construct_value_called_, 2);
    BOOST_CHECK_EQUAL(mock0.destruct_value_called_,  0);

  }

  sequence_inst.destruct_value(storage, &alloc);
}

BOOST_AUTO_TEST_SUITE_END()
