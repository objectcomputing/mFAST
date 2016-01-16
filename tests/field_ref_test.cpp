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

#include <mfast/int_ref.h>
#include <mfast/string_ref.h>
#include <mfast/decimal_ref.h>
#include <mfast/group_ref.h>
#include <mfast/sequence_ref.h>
#include <mfast/coder/common/codec_helper.h>
#include "debug_allocator.h"

using namespace mfast;


TEST_CASE("test the operations of interger_field","[integer_field_test]")
{
  malloc_allocator allocator;
  value_storage storage;

  uint64_cref null_ref;

  REQUIRE(!null_ref.present() );

  {
    uint64_field_instruction inst(operator_delta,
                                  presence_optional,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));

    REQUIRE(inst.initial_value().is_empty() ==  false);

    inst.construct_value(storage, &allocator);

    uint64_cref cref(&storage, &inst);

    REQUIRE(!cref.present() );
    REQUIRE(cref.id() ==  1U);
    REQUIRE( strcmp(cref.name(), "test_uint64") == 0);

    uint64_mref ref(&allocator, &storage, &inst);

    REQUIRE(!ref.present() );
    REQUIRE(ref.id() ==  1U);
    REQUIRE( strcmp(ref.name(), "test_uint64") == 0);

    ref.as(5);
    REQUIRE( ref.present() );
    REQUIRE(ref.value() ==  5U);

    ref.as(20150701121211999);
    value_storage storage2;
    inst.copy_construct_value(storage, storage2, &allocator);
    uint64_cref copied_cref(&storage2, &inst);

    REQUIRE(ref.value() == copied_cref.value());

    ref.to_initial_value();
    REQUIRE( ref.present() );
    REQUIRE(ref.value() ==  UINT64_MAX);

    // test convertion from field_mref to field_cref

    uint64_cref another_cref(ref);
    REQUIRE(another_cref.value() ==  UINT64_MAX);


    ref.omit();
    REQUIRE(!ref.present() );

    detail::codec_helper helper;

    REQUIRE(!helper.previous_value_of(ref).is_defined() );

    // The base value depends on the state of the previous value in the following way:
    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    // * empty – it is a dynamic error [ERR D6] if the previous value is empty.


    {
      value_storage base_value = helper.delta_base_value_of(ref);
      uint64_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE(base_cref.value() ==  UINT64_MAX);
    }

#ifndef EMSCRIPTEN
    {
      helper.save_previous_value(ref);
      REQUIRE_THROWS_AS(helper.delta_base_value_of(ref), mfast::fast_error);
    }
#endif
    {
      ref.as(4);
      helper.save_previous_value(ref);
      value_storage base_value = helper.delta_base_value_of(ref);
      uint64_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE(base_cref.value() ==  4U);
    }
  }

  {
    uint64_field_instruction inst(operator_copy,
                                  presence_mandatory,
                                  1,
                                  "test_uint64","",
                                  nullptr,
                                  int_value_storage<uint64_t>(UINT64_MAX));


    inst.construct_value(storage, &allocator);

    uint64_cref cref(&storage, &inst);

    REQUIRE(cref.present() );
    REQUIRE(cref.id() ==  1U);
    REQUIRE( strcmp(cref.name(), "test_uint64") == 0);
    REQUIRE( cref.value() ==  UINT64_MAX);

    uint64_mref ref(&allocator, &storage, &inst);
    REQUIRE(ref.present() );
    ref.omit(); // mandatory field shoudn't be able to be absent
    REQUIRE(ref.present() );




  }

  {
    int32_field_instruction inst(operator_copy,
                                 presence_mandatory,
                                 1,
                                 "test_int32","",
                                 nullptr,
                                 int_value_storage<int32_t>(INT32_MIN));

    inst.construct_value(storage, &allocator);

    int32_cref cref(&storage, &inst);
    REQUIRE(cref.present() );
    REQUIRE(cref.value() ==  INT32_MIN);
  }


  uint32_field_instruction inst(operator_copy,
                                presence_mandatory,
                                1,
                                "test_uint32","",
                                nullptr,
                                int_value_storage<uint32_t>());

  REQUIRE(inst.initial_value().is_empty() ==  true);

}

TEST_CASE("test the operations of decimal_field_instruction","[decimal_field_instruction_test]")
{
  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   decimal_value_storage(INT64_MAX,64));
    REQUIRE(inst.initial_value().is_empty() ==  false);
  }

  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   decimal_value_storage());
    REQUIRE(inst.initial_value().is_empty() ==  true);
  }
  {
    mantissa_field_instruction mantissa_inst(operator_copy,
                                             nullptr,
                                             int_value_storage<int64_t>());

    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   &mantissa_inst,
                                   decimal_value_storage());
    REQUIRE(inst.initial_value().is_empty() ==  true);
  }
  {
    mantissa_field_instruction mantissa_inst(operator_copy,
                                             nullptr,
                                             int_value_storage<int64_t>());

    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   &mantissa_inst,
                                   decimal_value_storage(0, 3));
    REQUIRE(inst.initial_value().is_empty() ==  false);
  }
}

TEST_CASE("test the operations of decimal_field without mantissa instruction","[decimal_field_test]")
{
  malloc_allocator allocator;
  value_storage storage;

  {
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal","",
                                   nullptr,
                                   decimal_value_storage(INT64_MAX,64));

    inst.construct_value(storage, &allocator);
    REQUIRE(inst.mantissa_instruction() == nullptr);

    decimal_cref cref(&storage, &inst);

    REQUIRE(!cref.present() );
    REQUIRE(cref.id() ==  1U);
    REQUIRE( strcmp(cref.name(), "test_decimal") == 0);

    decimal_mref ref(&allocator, &storage, &inst);

    REQUIRE(!ref.present() );
    REQUIRE(ref.id() ==  1U);
    REQUIRE( strcmp(ref.name(), "test_decimal") == 0);

    ref.as(5, 20);
    REQUIRE( ref.present() );
    REQUIRE(ref.mantissa() ==  5);
    REQUIRE(ref.exponent() ==  20);

    ref.to_initial_value();
    REQUIRE( ref.present() );
    REQUIRE(ref.mantissa() ==  INT64_MAX);
    REQUIRE(ref.exponent() ==  64);

    ref.omit();
    REQUIRE(!ref.present() );

    ref.set_mantissa(4);
    REQUIRE(ref.present() );
    REQUIRE(ref.mantissa() ==  4);
    REQUIRE(ref.exponent() ==  64);

    ref.omit();
    ref.set_exponent(5);
    REQUIRE(ref.present() );
    REQUIRE(ref.exponent() ==  5);


    // test convertion from field_mref to field_cref
    decimal_cref another_cref(ref);
    REQUIRE(another_cref.mantissa() ==  4);
    REQUIRE(another_cref.exponent() ==  5);

    // test delta base value

    detail::codec_helper helper;

    REQUIRE(!helper.previous_value_of(ref).is_defined() );

    // The base value depends on the state of the previous value in the following way:
    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    // * empty – it is a dynamic error [ERR D6] if the previous value is empty.

    {
      value_storage base_value = helper.delta_base_value_of(ref);
      decimal_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE(base_cref.mantissa() ==  INT64_MAX);
      REQUIRE(base_cref.exponent() ==  64);

    }
#ifndef EMSCRIPTEN
    {
      ref.omit();
      helper.save_previous_value(ref);
      REQUIRE_THROWS_AS(helper.delta_base_value_of(ref), mfast::fast_error);
    }
#endif
    {
      ref.set_mantissa(4);
      ref.set_exponent(0);
      helper.save_previous_value(ref);
      value_storage base_value = helper.delta_base_value_of(ref);
      decimal_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE(base_cref.mantissa() ==  4);
      REQUIRE(base_cref.exponent() ==  0);

    }

    // test conversion from decimal value

    ref.as(decimal(500));
    REQUIRE(ref.mantissa() ==  5);
    REQUIRE(ref.exponent() ==  2);

    ref.as(decimal("0.05"));
    REQUIRE(ref.mantissa() ==  5);
    REQUIRE(ref.exponent() ==  -2);

    ref.as(-110);
    REQUIRE(ref.value() ==  decimal(-110));

    ref.as("123.45");
    REQUIRE(ref.mantissa() ==  12345);
    REQUIRE(ref.exponent() ==  -2);
  }
}

TEST_CASE("test the operations of decimal_field with mantissa instruction","[decimal_field_test2]")
{
  malloc_allocator allocator;
  value_storage storage;

  {
    mantissa_field_instruction mantissa_inst(operator_copy, nullptr, int_value_storage<int64_t>(6));
    decimal_field_instruction inst(operator_copy,
                                   presence_optional,
                                   1,
                                   "test_decimal2","",
                                   nullptr,
                                   &mantissa_inst,
                                   decimal_value_storage(0,64));

    inst.construct_value(storage, &allocator);

    decimal_cref cref(&storage, &inst);

    REQUIRE(!cref.present() );
    REQUIRE(cref.id() ==  1U);
    REQUIRE( strcmp(cref.name(), "test_decimal2") == 0);

    decimal_mref ref(&allocator, &storage, &inst);

    REQUIRE(!ref.present() );
    REQUIRE(ref.id() ==  1U);
    REQUIRE( strcmp(ref.name(), "test_decimal2") == 0);

    ref.as(5, 20);
    REQUIRE( ref.present() );
    REQUIRE(ref.mantissa() ==  5);
    REQUIRE(ref.exponent() ==  20);

    ref.to_initial_value();
    REQUIRE( ref.present() );
    REQUIRE(ref.mantissa() ==  6);
    REQUIRE(ref.exponent() ==  64);

    ref.omit();
    REQUIRE(!ref.present() );

    ref.set_mantissa(4);
    REQUIRE(ref.present() );
    REQUIRE(ref.mantissa() ==  4);
    REQUIRE(ref.exponent() ==  64);

    ref.omit();
    ref.set_exponent(5);
    REQUIRE(ref.present() );
    REQUIRE(ref.exponent() ==  5);

    exponent_mref exp_mref = ref.set_exponent();
    exp_mref.to_initial_value();
    REQUIRE(exp_mref.value() ==  64);
    REQUIRE(ref.exponent() ==    64);

  }

  {
    mantissa_field_instruction mantissa_inst(operator_copy, nullptr, int_value_storage<int64_t>(6));
    decimal_field_instruction inst(operator_default,
                                   presence_optional,
                                   1,
                                   "test_decimal2","",
                                   nullptr,
                                   &mantissa_inst,
                                   decimal_value_storage());

    inst.construct_value(storage, &allocator);

    decimal_mref ref(&allocator, &storage, &inst);
    ref.as(1,1); // set the value so it's present

    // The exponent initial value is absent, therefore, the entire value should be absent().
    ref.set_exponent().to_initial_value();
    REQUIRE(ref.absent());
  }

  {
    mantissa_field_instruction mantissa_inst(operator_copy, nullptr, int_value_storage<int64_t>(6));
    decimal_field_instruction inst(operator_constant,
                                   presence_mandatory,
                                   1,
                                   "test_decimal3","",
                                   nullptr,
                                   &mantissa_inst,
                                   decimal_value_storage(0, -1));

    inst.construct_value(storage, &allocator);

    decimal_mref ref(&allocator, &storage, &inst);
    ref.as(1,1); // set the value so it's present

    ref.normalize();
    REQUIRE(ref.mantissa() ==  100);
    REQUIRE(ref.exponent() ==  -1);

    ref.as(12345, -3);
    ref.normalize();
    REQUIRE(ref.mantissa() ==  123);
    REQUIRE(ref.exponent() ==  -1);

  }
  {
    mfast::decimal_type x;
    x.mref ().as (9664.3);

    double y(x.cref());
    REQUIRE(y == Approx( 9664.3));

    x.mref ().as (0);
    REQUIRE(x.cref().mantissa() ==  0LL);
  }

}

TEST_CASE("test the operations of string_field_instruction","[string_field_instruction_test]")
{
  debug_allocator alloc;
  value_storage storage;
  const char* default_value = "initial_string";
  {
    ascii_field_instruction inst(operator_copy,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage(default_value));

    REQUIRE(inst.initial_value().is_empty() ==  false);
  }
  {
    ascii_field_instruction inst(operator_copy,
                                 presence_optional,
                                 1,
                                 "test_ascii","",
                                 nullptr,
                                 string_value_storage());

    REQUIRE(inst.initial_value().is_empty() ==  true);
  }
}

TEST_CASE("test the operations of string_field","[string_field_test]")
{
  debug_allocator alloc;
  value_storage storage;
  const char* default_value = "initial_string";
  ascii_field_instruction inst(operator_copy,
                               presence_optional,
                               1,
                               "test_ascii","",
                               nullptr,
                               string_value_storage(default_value));

  inst.construct_value(storage, &alloc);
  REQUIRE(inst.initial_value().is_empty() ==      false);

  REQUIRE(storage.of_array.capacity_in_bytes_ ==  0U);

  {
    ascii_string_cref cref(&storage, &inst);
    REQUIRE(!cref.present() );
    REQUIRE(cref.id() ==  1U);
    REQUIRE( strcmp(cref.name(), "test_ascii") == 0);
  }

  {
    ascii_string_mref ref(&alloc, &storage, &inst);
    REQUIRE(!ref.present());

    ref.push_back('c');
    REQUIRE(ref.size() ==  1U);

    ref.to_initial_value();
    REQUIRE(ref.present());
    REQUIRE(ref.size() ==  strlen(default_value));
    REQUIRE(ref == "initial_string" );

    // testing value
    std::stringstream strm;
    strm << ref.value();
    REQUIRE(strm.str() ==  std::string("initial_string"));

    ref.as("string1");
    REQUIRE(ref == "string1" );

    ref.as("string2");
    REQUIRE(ref == "string2" );

    ref += "abc";
    REQUIRE(ref == "string2abc" );

    ref.resize(1);
    REQUIRE(ref == "s");

    ref.replace(0, 0, "abcd", 4);
    REQUIRE(ref == "abcds");

    ref.replace(0, 2, "xxxx", 4);
    REQUIRE(ref == "xxxxcds");

    ref.replace(4, 3, "yy", 2);
    REQUIRE(ref == "xxxxyy");

    ref.replace(4, 2, "aaaa", 4);
    REQUIRE(ref == "xxxxaaaa");

    ref.refers_to("abcde");
    REQUIRE(ref == "abcde");
    REQUIRE(storage.of_array.capacity_in_bytes_ ==  0U);

    ref.pop_back();
    REQUIRE(ref == "abcd");

    ref.clear();
    REQUIRE(ref.absent());
    ref.as("cde");
    REQUIRE(ref == "cde");

    ref.omit();
    REQUIRE(ref.absent());



    detail::codec_helper helper;

    REQUIRE(!helper.previous_value_of(ref).is_defined() );


    // testing delta base value

    // The base value depends on the state of the previous value in the following way:
    // * assigned – the base value is the previous value.
    // * undefined – the base value is the initial value if present in the instruction context. Otherwise a type dependant default base value is used.
    // * empty – it is a dynamic error [ERR D6] if the previous value is empty.

    {
      value_storage base_value = helper.delta_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE( base_cref == "initial_string" );
    }
#ifndef EMSCRIPTEN
    {
      helper.save_previous_value(ref);
      REQUIRE_THROWS_AS(helper.delta_base_value_of(ref), mfast::fast_error);
    }
#endif
    {
      ref.as("4");
      helper.save_previous_value(ref);
      value_storage base_value = helper.delta_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE( base_cref == "4");
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
      REQUIRE( base_cref.present() );
      REQUIRE( base_cref == "4");
    }
    ref.omit();
    helper.save_previous_value(ref);
    {
      value_storage base_value = helper.tail_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE( base_cref == "initial_string" );
    }

    helper.previous_value_of(ref).defined(false);

    {
      value_storage base_value = helper.tail_base_value_of(ref);
      ascii_string_cref base_cref(&base_value, &inst);
      REQUIRE( base_cref.present() );
      REQUIRE( base_cref == "initial_string" );
    }

  }


  {
    // testing swap operation

    ascii_string_type string_holder1(&alloc);
    ascii_string_type string_holder2(&alloc);

    string_holder1.mref().as("abc");

    REQUIRE(string_holder1.cref().size() ==  3U);
    REQUIRE(boost::string_ref("abc") ==  string_holder1.cref().value());


    string_holder2.mref().as("def");
    string_holder1.mref().swap(string_holder2.mref());

    REQUIRE(boost::string_ref("def") ==  string_holder1.cref().value());
    REQUIRE(boost::string_ref("abc") ==  string_holder2.cref().value());
  }

  inst.destruct_value(storage, &alloc);

}



TEST_CASE("test string with delta instruction","[string_delta_test]")
{
  debug_allocator alloc;
  value_storage storage;
  const char* default_value = "initial_string";
  ascii_field_instruction inst(operator_copy,
                               presence_optional,
                               1,
                               "test_ascii","",
                               nullptr,
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
    REQUIRE( mref.size() == std::strlen(result));
    REQUIRE( std::equal(result, result+10, mref.data()));


    helper.apply_string_delta(mref, base_value, -5, delta_str, delta_len);
    result = "ABCDE_value";
    REQUIRE( mref.size() == std::strlen(result));
    REQUIRE( std::equal(result, result+11, mref.data()) );
  }

  inst.destruct_value(storage, &alloc);

}


TEST_CASE("test the operations of group field","[group_field_test]")
{
  debug_allocator alloc;
  value_storage storage;

  unsigned char f0_initial[] = "\x01\x02\x03\x04\x05";
  const char* f1_initial = "abcdefg";

  byte_vector_field_instruction inst0(operator_copy,
                                      presence_optional,
                                      1,
                                      "test_byte_vector","",
                                      nullptr,
                                      byte_vector_value_storage(f0_initial,sizeof(f0_initial)),
                                      0, nullptr, nullptr);

  unicode_field_instruction inst1(operator_copy,
                                  presence_optional,
                                  2,
                                  "test_unicode","",
                                  nullptr,
                                  string_value_storage(f1_initial), 0, "", "");

  const field_instruction* instructions[] = {
    &inst0,&inst1
  };

  group_field_instruction group_inst(presence_optional,
                                     3,
                                     "test_group","","",
                                     instructions,
                                     "", "", "");

  REQUIRE(group_inst.group_content_byte_count() ==  2 * sizeof(value_storage) );

  group_inst.construct_value(storage, &alloc);

  REQUIRE(storage.of_group.content_ != nullptr);
  {
    group_cref ref(&storage, &group_inst);
    REQUIRE(ref.present() ==                          false);
    REQUIRE(ref.num_fields() ==                       2U);

    REQUIRE(aggregate_cref(ref).subinstruction(0) ==  &inst0);
    REQUIRE(aggregate_cref(ref).subinstruction(1) ==  &inst1);

    field_cref f0( ref[0] );
    REQUIRE(f0.absent());

    field_cref f1( ref[1] );
    REQUIRE(f1.absent());


    byte_vector_cref cf0(f0);
    REQUIRE(cf0.field_type() ==  field_type_byte_vector);

    unicode_string_cref cf1(f1);
    REQUIRE(cf1.field_type() ==  field_type_unicode_string);
  }

  {
    group_mref ref(&alloc, &storage, &group_inst);
    REQUIRE(ref.present() ==     true);
    REQUIRE(ref.num_fields() ==  2U);

    field_mref f0(ref[0] );
    REQUIRE(f0.absent());

    REQUIRE(ref.present());

    field_mref f1( ref[1] );
    REQUIRE(f1.absent());


    byte_vector_mref mf0(f0);
    REQUIRE(mf0.field_type() ==  field_type_byte_vector);

    unicode_string_mref mf1(f1);
    REQUIRE(mf1.field_type() ==  field_type_unicode_string);

    mf0.to_initial_value();

    REQUIRE(mf0.size() ==                                  sizeof(f0_initial));
    REQUIRE(memcmp(mf0.data(),  f0_initial, mf0.size()) == 0);

    unsigned char bytes[] = "test";
    mf0.assign(bytes, bytes+4);

    REQUIRE(mf0.size() ==                             4U);
    REQUIRE(memcmp(mf0.data(),  bytes, mf0.size()) == 0);

    mf0.refers_to(bytes, 4);
    REQUIRE(mf0.data() ==  bytes);
    REQUIRE(mf0.size() ==  4U);
  }

  group_inst.destruct_value(storage, &alloc);

}

TEST_CASE("test the operations of sequence field","[sequence_field_test]")
{
  debug_allocator alloc;
  value_storage storage;

  unsigned char f0_initial[] = "\x01\x02\x03\x04\x05";
  const char* f1_initial = "abcdefg";

  byte_vector_field_instruction inst0(operator_copy,
                                      presence_optional,
                                      1,
                                      "test_byte_vector","",
                                      nullptr,
                                      byte_vector_value_storage(f0_initial,sizeof(f0_initial)),
                                      0, nullptr, nullptr);

  unicode_field_instruction inst1(operator_copy,
                                  presence_optional,
                                  2,
                                  "test_unicode","",
                                  nullptr,
                                  string_value_storage(f1_initial), 0, "", "");

  uint32_field_instruction length_inst(operator_none,
                                       presence_mandatory,
                                       4,
                                       "","",
                                       nullptr,
                                       2);


  const field_instruction* instructions[] = {
    &inst0,&inst1
  };

  sequence_field_instruction sequence_inst(presence_optional,
                                           3, // id
                                           "test_group","","",
                                           instructions,nullptr,nullptr,
                                           &length_inst, "", "", "");

  REQUIRE(sequence_inst.subinstructions().size() ==     2U);
  REQUIRE(sequence_inst.group_content_byte_count() ==  2 * sizeof(value_storage) );

  sequence_inst.construct_value(storage, &alloc);

  {
    sequence_mref ref(&alloc, &storage, &sequence_inst);
    REQUIRE(ref.present() ==     false);
    REQUIRE(ref.num_fields() ==  2U);

    ref.resize(2);
    REQUIRE(ref.present() ==     true);

    ref.omit();
    REQUIRE(ref.present() ==     false);

    ref.resize(2);
    REQUIRE(ref.present() ==     true);

    sequence_element_mref e0ref(ref[0]);
    sequence_element_cref e0cref(ref[0]);


    field_mref e0f0(e0ref[0] );
    REQUIRE(e0f0.absent());

    field_mref e0f1( e0ref[1] );
    REQUIRE(e0f1.absent());

    {
      byte_vector_mref mf0(e0f0);
      REQUIRE(mf0.field_type() ==  field_type_byte_vector);

      unicode_string_mref mf1(e0f1);
      REQUIRE(mf1.field_type() ==  field_type_unicode_string);

      mf0.to_initial_value();

      REQUIRE(mf0.size() ==                                  sizeof(f0_initial));
      REQUIRE(memcmp(mf0.data(), f0_initial, mf0.size()) == 0);

      unsigned char bytes[] = "test";
      mf0.assign(bytes, bytes+4);

      REQUIRE(mf0.size() ==                             4U);
      REQUIRE(memcmp(mf0.data(), bytes, mf0.size()) == 0);
    }

    ref.resize(4);
    REQUIRE(ref.size() ==  4U);
    {
      field_mref e3f1(ref[3][1] );
      REQUIRE(e3f1.absent());

      field_cref const_e3f1(ref[3][1] );
      REQUIRE(const_e3f1.absent());

      unicode_string_mref str(e3f1);
      REQUIRE(str.field_type() ==  field_type_unicode_string);

      str.as( "abcdef" );
      REQUIRE(const_e3f1.present());

      unicode_string_cref cstr(e3f1);
      REQUIRE(cstr == str);
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
                               allocator*       /* alloc */) const override
  {
    construct_value_called_++;
  }

  virtual void destruct_value(value_storage& /* storage */,
                              allocator*       /* alloc */) const override
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

  virtual void accept(field_instruction_visitor&, void*) const override
  {
  }

  virtual mock_field_instruction* clone(arena_allocator& alloc) const override
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

  virtual std::size_t reallocate(void*& pointer, std::size_t old_size, std::size_t new_size) override
  {
    void* old_ptr = pointer;
    if (pointer) {
      pointer = std::realloc(pointer, new_size);
      std::memset(static_cast<char*>(pointer)+old_size, 0xFF, new_size-old_size);
    }
    else
      pointer = std::malloc(new_size);
    leased_addresses_.erase(old_ptr);
    if (pointer == nullptr) {
      std::free(old_ptr);
      throw std::bad_alloc();
    }
    leased_addresses_.insert(pointer);
    return new_size;
  }

};


TEST_CASE("test sequence_mref.resize()","[sequence_resize_test]")
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
                                       nullptr,
                                       int_value_storage<uint32_t>(0));

  sequence_field_instruction sequence_inst(presence_optional,
                                           3, // id
                                           "test_group","","",
                                           instructions,nullptr,nullptr,
                                           &length_inst, "", "", "");

  sequence_inst.construct_value(storage, &alloc);


  {
    sequence_mref ref(&alloc, &storage, &sequence_inst);
    REQUIRE(ref.present() ==                  false);
    REQUIRE(ref.num_fields() ==               1U);

    ref.reserve(2);
    REQUIRE(ref.size() ==                     0U);
    REQUIRE(mock0.construct_value_called_ ==  2);
    REQUIRE(mock0.destruct_value_called_ ==   0);


    ref.resize(2);
    REQUIRE(ref.present() ==                  true);
    REQUIRE(ref.size() ==                     2U);

    REQUIRE(mock0.construct_value_called_ ==  2);
    REQUIRE(mock0.destruct_value_called_ ==   0);


    ref.resize(1);
    REQUIRE(mock0.construct_value_called_ ==  2);
    REQUIRE(mock0.destruct_value_called_ ==   0);

  }

  sequence_inst.destruct_value(storage, &alloc);
}

