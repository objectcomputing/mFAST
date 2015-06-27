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

#include "test3.h"
#include <mfast/json/json.h>
#include <sstream>
#include "debug_allocator.h"


namespace mfast {
namespace json {
bool get_quoted_string(std::istream&                  strm,
                       std::string*                   pstr,
                       const mfast::byte_vector_mref* pref,
                       bool                           first_quote_extracted=false);
bool skip_value (std::istream& strm);


namespace encode_detail {

struct quoted_string {
  quoted_string(const char* str)
    : str_(str)
  {
  }

  const char* str_;
};


std::ostream& operator << (std::ostream& os, quoted_string str);


}
}
}



TEST_CASE("test json_encode with product type","[json_encode_product_test]")
{
  using namespace test3;

  Product product_holder;
  Product_mref product_ref = product_holder.mref();

  // product_ref.set_id().as(1);
  product_ref.set_price().as(12356, -2);
  product_ref.set_name().as("Foo");
  Product_mref::tags_mref tags = product_ref.set_tags();
    REQUIRE(tags.instruction()->field_type() ==              mfast::field_type_sequence);
    REQUIRE(tags.instruction()->subinstructions().size() ==  1U);

  tags.resize(2);
    REQUIRE(tags.size() ==                                   2U);

  mfast::ascii_string_mref tag0 = tags[0];
    REQUIRE(tag0.instruction()->field_type() ==              mfast::field_type_ascii_string);


  tags[0].as("Bar with \"quote\"");
    REQUIRE(strcmp(tags[0].c_str(),  "Bar with \"quote\"") == 0);
  tags[1].as("Eek with \\");
    REQUIRE(strcmp(tags[1].c_str(),  "Eek with \\") == 0);

  Product_mref::stock_mref stock = product_ref.set_stock();
  stock.set_warehouse().as(300);
  stock.set_retail().as(20);

  const unsigned char ext_data[] = "{\"test1\":1}";
  // product_ref.set_ext().assign(ext_data, ext_data+sizeof(ext_data)-1);

  product_ref.set_ext().refers_to(ext_data, sizeof(ext_data)-1);
  const char* result = "{\"name\":\"Foo\",\"price\":123.56,\"tags\":[\"Bar with \\\"quote\\\"\",\"Eek with \\\\\"],\"stock\":{\"warehouse\":300,\"retail\":20},\"ext\":{\"test1\":1}}";

  {
    std::ostringstream ostrm;
    mfast::json::encode(ostrm,
                        product_ref,
                        mfast_tag::JSON_UNKNOWN);


    // std::cout << strm.str() << "\n";
    // std::cout << result << "\n";

    REQUIRE(ostrm.str() == result);
  }
  {

    std::ostringstream ostrm;
    mfast::json::encode(ostrm,
                        product_ref,
                        mfast_tag::JSON_UNKNOWN,
                        mfast_tag::JSON_IGNORE);

    const char* result = "{\"name\":\"Foo\",\"price\":123.56,\"stock\":{\"warehouse\":300,\"retail\":20},\"ext\":{\"test1\":1}}";

    REQUIRE(ostrm.str() == result);
  }

  debug_allocator alloc;
  Product product2_holder(product_ref, &alloc);

  Product product3_holder;
  std::istringstream istrm(result);
  mfast::json::decode(istrm,
                      product3_holder.mref(),
                      mfast_tag::JSON_UNKNOWN);
  //
  REQUIRE(product3_holder.cref() == product_ref);

  product_ref.omit_stock();
  REQUIRE(product_ref.get_stock().absent());
  REQUIRE_THROWS_AS(product_ref.try_get_stock(), mfast::bad_optional_access);
}

TEST_CASE("test json_encode with person type","[json_encode_person_test]")
{
  using namespace test3;

  Person person_holder;
  Person_mref person_ref = person_holder.mref();

  person_ref.set_firstName().as("John");
  person_ref.set_lastName().as("Smith");
  person_ref.set_age().as(25);
  Person_mref::phoneNumbers_mref phones = person_ref.set_phoneNumbers();
  phones.resize(2);
  phones[0].set_type().as("home");
  phones[0].set_number().as("212 555-1234");

  phones[1].set_type().as("fax");
  phones[1].set_number().as("646 555-4567");

  LoginAccount_mref login = person_ref.set_login().as<LoginAccount>();
  login.set_userName().as("John");
  login.set_password().as("J0hnsm1th");

  REQUIRE(person_ref.get_login().present());

  person_ref.set_bankAccounts().grow_by(1);
  REQUIRE(person_ref.get_bankAccounts().size() ==  1U);

  BankAccount_mref acct0 = person_ref.set_bankAccounts()[0].as<BankAccount>();
  acct0.set_number().as(12345678);
  acct0.set_routingNumber().as(87654321);


  REQUIRE(person_ref.get_bankAccounts().size() ==  1U);
  mfast::nested_message_cref n0 = person_ref.get_bankAccounts()[0];
  BankAccount_cref acct0_read = static_cast<BankAccount_cref>(n0.target());

  REQUIRE(acct0_read.get_number().value() ==         12345678U);
  REQUIRE(acct0_read.get_routingNumber().value() ==  87654321U);


  std::stringstream strm;
  mfast::json::encode(strm, person_ref);

  auto result = R"({"firstName":"John","lastName":"Smith","age":25,)"
                R"("phoneNumbers":[{"type":"home","number":"212 555-1234"},{"type":"fax","number":"646 555-4567"}],)"
                R"("emails":[],"login":{"userName":"John","password":"J0hnsm1th"},"bankAccounts":[{"number":12345678,"routingNumber":87654321}]})";

  REQUIRE(strm.str() == result);

  debug_allocator alloc;

  Person person_holder2(person_ref, &alloc);
}

TEST_CASE("test json_decode with a value null","[json_decode_null_test]")
{
  using namespace test3;

  try {
    LoginAccount account_holder;

    auto& result = R"({"userName":"test","password": null})";
    std::stringstream strm(result);

    mfast::json::decode(strm, account_holder.mref());
    REQUIRE(account_holder.cref().get_userName().value() ==  boost::string_ref("test"));
    REQUIRE( account_holder.cref().get_password().absent() );
  }
  catch (boost::exception& ex)
  {
    std::cerr << diagnostic_information(ex);
  }
}



TEST_CASE("test the json encoding with quoted string","[test_encode_quoted_string]")
{
  {
    std::stringstream stream;
    mfast::json::encode_detail::quoted_string str("abcd\x01\b\t\f\n\r\\\"");
    stream << str;
    REQUIRE(stream.str() ==  std::string("\"abcd\\u0001\\b\\t\\f\\n\\r\\\\\\\"\"") );
  }

  {
    std::stringstream stream;
    mfast::json::encode_detail::quoted_string str("\xE2\x80\x93");
    stream << str;
    REQUIRE(stream.str() ==  std::string("\"\xE2\x80\x93\"") );
  }
}

TEST_CASE("test the json decoding with quoted string","[test_get_quoted_string]")
{
  using namespace mfast;
  using namespace mfast::json;
  debug_allocator alloc;

  std::string str;

  const byte_vector_field_instruction byte_vector_field_instruction_prototype(operator_none,presence_mandatory,0,nullptr,"",nullptr, string_value_storage(), 0, "", "");
  value_storage storage;

  byte_vector_field_instruction_prototype.construct_value(storage, &alloc);

  byte_vector_mref bv_ref(&alloc,
                          &storage,
                          &byte_vector_field_instruction_prototype);


  {
    const char data[] = R"("abcd",)";
    std::stringstream strm(data);
    REQUIRE(get_quoted_string(strm, &str, &bv_ref, false));
    REQUIRE(str ==            std::string("abcd"));

    REQUIRE(bv_ref.size() ==  sizeof(data)-2 );
    REQUIRE(memcmp(data, bv_ref.data(), sizeof(data)-2 ) ==0);

    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  {
    bv_ref.clear();
    const char data[] = R"("abc\"d",)";
    std::stringstream strm(data);
    REQUIRE(get_quoted_string(strm, &str, &bv_ref, false));
    REQUIRE(str ==            std::string(R"(abc"d)") );

    REQUIRE(bv_ref.size() ==  sizeof(data)-2 );
    REQUIRE(memcmp(data, bv_ref.data(), sizeof(data)-2) == 0);

    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  {
    bv_ref.clear();
    const char data[] = R"("\u4e2d\u83EF\u6c11\u570B",)";
    std::stringstream strm(data);
    REQUIRE(get_quoted_string(strm, &str, &bv_ref, false));
    REQUIRE(str ==            std::string("中華民國"));

    REQUIRE(bv_ref.size() ==  sizeof(data)-2 );
    REQUIRE(memcmp(data, bv_ref.data(), sizeof(data)-2 ) ==0);

    strm >> str;
    REQUIRE(str ==  std::string(","));
  }

  {
    bv_ref.clear();
    // U+10437 U+24B62
    const char data[] = R"("\uD801\uDC37\uD852\uDF62",)";
    std::stringstream strm(data);
    REQUIRE(get_quoted_string(strm, &str, &bv_ref, false));
    REQUIRE(str ==            std::string("\xF0\x90\x90\xB7\xF0\xA4\xAD\xA2"));

    REQUIRE(bv_ref.size() ==  sizeof(data)-2 );
    REQUIRE(memcmp(data, bv_ref.data(), sizeof(data)-2 ) ==0);

    strm >> str;
    REQUIRE(str ==  std::string(","));
  }

  // {
  //     bv_ref.clear();
  //     const char data[] = "\"Konu\\u0015fal\\u00131m bir gün\",";
  //     std::stringstream strm(data);
  //     REQUIRE(get_quoted_string(strm, &str, &bv_ref, false));
  // }
  {
    bv_ref.clear();
    const char data[] = "\"abc\\nd\\/\",";
    std::stringstream strm(data);
    REQUIRE(get_quoted_string(strm, &str, &bv_ref, false));
    REQUIRE(str ==            std::string("abc\nd/"));

    REQUIRE(bv_ref.size() ==  sizeof(data)-2);
    REQUIRE(memcmp(data, bv_ref.data(), sizeof(data)-2) ==0);

    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  byte_vector_field_instruction_prototype.destruct_value(storage, &alloc);

}


TEST_CASE("test json skip value","[test_skip_value]")
{
  using namespace mfast::json;
  std::string str;
  {
    std::stringstream strm(" 123.45,");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  {
    std::stringstream strm(" \"ab\\ncd\",");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  {
    std::stringstream strm(" null,");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  {
    std::stringstream strm(" true,");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  {
    std::stringstream strm(" false,");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
  }
  {
    std::stringstream strm(" [1, 2, 3],");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
    REQUIRE(strm.eof());
  }
  {
    std::stringstream strm(R"( [1, [ "abc" ], 3],)");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
    REQUIRE(strm.eof());
  }
  {
    std::stringstream strm(R"( [1, { "f1":"abc" }, 3],)");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
    REQUIRE(strm.eof());
  }
  {
    std::stringstream strm(R"( {"id":1,"name":"Foo"},)");
    REQUIRE(skip_value(strm));
    strm >> str;
    REQUIRE(str ==  std::string(","));
    REQUIRE(strm.eof());
  }
}

TEST_CASE("test code generation for sequence","[test_seq_codegen]")
{
  using namespace test3;

  const UsingSeqTemplates::instruction_type* top_inst = UsingSeqTemplates::instruction();
  REQUIRE(top_inst->subinstructions().size() ==  3U);

  const mfast::sequence_field_instruction* seq1_inst = dynamic_cast<const mfast::sequence_field_instruction*>(top_inst->subinstruction(1));
  REQUIRE(seq1_inst);
  REQUIRE(strcmp(seq1_inst->name(), "seq1")==0);
  REQUIRE(seq1_inst->subinstructions().size() ==  2U);
  REQUIRE(seq1_inst->ref_instruction() ==         SeqTemplate1::instruction());
  REQUIRE(seq1_inst->element_instruction() ==     (const mfast::group_field_instruction*) nullptr);


  const mfast::sequence_field_instruction* seq2_inst = dynamic_cast<const mfast::sequence_field_instruction*>(top_inst->subinstruction(2));
  REQUIRE(seq2_inst);
  REQUIRE(strcmp(seq2_inst->name(), "seq2")==0);
  REQUIRE(seq2_inst->subinstructions().size() ==  4U);
  REQUIRE(seq2_inst->ref_instruction() ==         SeqTemplate2::instruction());
  REQUIRE(seq2_inst->element_instruction() ==     BankAccount::instruction());
}


TEST_CASE("test decimal output","[test_decimal_output]")
{
  using mfast::decimal_value_storage;

  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, 0);
    REQUIRE(strm.str() ==  std::string("12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, 0);
    REQUIRE(strm.str() ==  std::string("-12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, 2);
    REQUIRE(strm.str() ==  std::string("1234500"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, 2);
    REQUIRE(strm.str() ==  std::string("-1234500"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, -2);
    REQUIRE(strm.str() ==  std::string("123.45"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, -2);
    REQUIRE(strm.str() ==  std::string("-123.45"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, -5);
    REQUIRE(strm.str() ==  std::string("0.12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, -5);
    REQUIRE(strm.str() ==  std::string("-0.12345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(12345, -7);
    REQUIRE(strm.str() ==  std::string("0.0012345"));
  }
  {
    std::stringstream strm;
    strm << decimal_value_storage(-12345, -7);
    REQUIRE(strm.str() ==  std::string("-0.0012345"));
  }
}

TEST_CASE("test decimal input","[test_decimal_input]")
{
  using namespace mfast::json;
  std::string str;
  mfast::decimal_value_storage storage;
  {
    std::stringstream strm(" 123.45,");
    strm >> storage;
    REQUIRE(storage.mantissa() ==  12345LL);
    REQUIRE(storage.exponent() ==  -2);
    strm >> str;
    REQUIRE(str ==                 std::string(","));
  }
  {
    std::stringstream strm(" -123.45,");
    strm >> storage;
    REQUIRE(storage.mantissa() ==  -12345LL);
    REQUIRE(storage.exponent() ==  -2);
    strm >> str;
    REQUIRE(str ==                 std::string(","));
  }
  {
    std::stringstream strm(" 1.2345E+12,");
    strm >> storage;
    REQUIRE(storage.mantissa() ==  12345LL);
    REQUIRE(storage.exponent() ==  8);
    strm >> str;
    REQUIRE(str ==                 std::string(","));
  }
  {
    std::stringstream strm(" 1.2345E-12,");
    strm >> storage;
    REQUIRE(storage.mantissa() ==  12345LL);
    REQUIRE(storage.exponent() ==  -16);
    strm >> str;
    REQUIRE(str ==  ",");
  }
  {
    std::stringstream strm(" 0.123,");
    strm >> storage;
    REQUIRE(storage.mantissa() ==  123LL);
    REQUIRE(storage.exponent() ==  -3);
    strm >> str;
    REQUIRE(str ==  ",");
  }
}


