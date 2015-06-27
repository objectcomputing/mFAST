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
#include <mfast/xml_parser/dynamic_templates_description.h>
#include <mfast/coder/common/template_repo.h>
#include <mfast/field_instructions.h>
#include <cstring>
#include <stdexcept>

#include"byte_stream.h"
#include"debug_allocator.h"

using namespace mfast;
namespace {
bool
same_dict(const field_instruction* lhs, const field_instruction* rhs)
{
  const ascii_field_instruction* lhs_inst = static_cast<const ascii_field_instruction*>(lhs);
  const ascii_field_instruction* rhs_inst = static_cast<const ascii_field_instruction*>(rhs);

  if (&lhs_inst->prev_value() == &rhs_inst->prev_value())
    return true;
  return false;
}

bool
different_dict(const field_instruction* lhs, const field_instruction* rhs)
{
  const ascii_field_instruction* lhs_inst = static_cast<const ascii_field_instruction*>(lhs);
  const ascii_field_instruction* rhs_inst = static_cast<const ascii_field_instruction*>(rhs);

  if (&lhs_inst->prev_value() != &rhs_inst->prev_value())
    return true;
  return false;
}


mfast::simple_template_repo_t repo;


const field_instruction* instruction_of(int template_id, const char* fieldname)
{
  template_instruction* inst =  repo.get_template(template_id);
  int field_index = inst->find_subinstruction_index_by_name(fieldname);
  return inst->subinstruction(field_index);
}
}



TEST_CASE("test the construction of fast dictionary", "[typeRef_test]")
{
  const char* xml_content =
   "<?xml version=\" 1.0 \"?>\n"
   "<templates xmlns=\"http://www.fixprotocol.org/ns/template-definition\""
   "templateNs=\"http://www.fixprotocol.org/ns/templates/sample\" ns=\"http://www.fixprotocol.org/ns/fix\">\n"
   "<template name=\"T1\" id=\"1\">"
   "<typeRef name=\"t4\" />"
     "<string name=\"Field1\" ><copy /> </string>"
     "<string name=\"Field2\" ><copy dictionary=\"global\"/> </string>"
     "<string name=\"Field3\" ><copy dictionary=\"type\"/> </string>"
     "<string name=\"Field4\" ><copy dictionary=\"template\"/> </string>"
     "<string name=\"Field5\" ><copy dictionary=\"template\"/></string>"
     "<string name=\"Field8\" ><copy dictionary=\"template\" key=\"Field4\"/></string>"
   "</template>"
   "<template name=\"T2\" id=\"2\">"
   "<typeRef name=\"t4\" />"
     "<string name=\"Field2\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field3\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field4\" ><copy dictionary=\"type\"/></string>"
     "<string name=\"Field5\" ><copy dictionary=\"template\"/></string>"
   "</template>"
   "<template name=\"T3\" id=\"3\">"
   "<typeRef name=\"t4\" />"
     "<string name=\"Field3\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field4\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field5\" ><copy dictionary=\"type\"/></string>"
     "<string name=\"Field6\" ><copy dictionary=\"template\"/></string>"
     "</template>"
   "<template name=\"T4\" id=\"4\">"
   "<typeRef name=\"t4\" />"
     "<string name=\"Field4\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field5\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field6\" ><copy dictionary=\"type\"/></string>"
     "<string name=\"Field7\" ><copy dictionary=\"template\"/></string>"
   "</template>"
   "<template name=\"T5\" id=\"5\"  dictionary=\"type\">"
   "<typeRef name=\"t4\" />"
     "<string name=\"Field1\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field2\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field3\" ><copy/></string>"
     "<string name=\"Field4\" ><copy/></string>"
     "<string name=\"Field5\" ><copy/></string>"
     "<string name=\"Field6\" ><copy/></string>"
     "<string name=\"Field7\" ><copy/></string>"
   "</template>"
   "<template name=\"T6\" id=\"6\" dictionary=\"type\">"
   "<typeRef name=\"t6\" />"
     "<string name=\"Field1\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field2\" ><copy dictionary=\"global\"/></string>"
     "<string name=\"Field3\" ><copy dictionary=\"type\"/></string>"
     "<string name=\"Field4\" ><copy dictionary=\"type\"/></string>"
     "<string name=\"Field5\" ><copy dictionary=\"type\"/></string>"
     "<string name=\"Field6\" ><copy dictionary=\"type\"/></string>"
     "<string name=\"Field7\" ><copy dictionary=\"type\"/></string>"
   "</template>"
   "</templates>\n";


  dynamic_templates_description description(xml_content);
  repo.build(&description ,&description+1);

  REQUIRE(same_dict(instruction_of(1, "Field1"), instruction_of(6, "Field1"))); // implicit vs explicit global
  REQUIRE(different_dict(instruction_of(1, "Field1"), instruction_of(1, "Field2"))); // different key
  REQUIRE(different_dict(instruction_of(2, "Field2"), instruction_of(2, "Field3"))); // different key


  REQUIRE(same_dict(instruction_of(1, "Field3"), instruction_of(5, "Field3"))); // type
  REQUIRE(same_dict(instruction_of(2, "Field3"), instruction_of(3, "Field3"))); // global
  REQUIRE(different_dict(instruction_of(1, "Field3"), instruction_of(3, "Field3"))); // type vs global
  REQUIRE(different_dict(instruction_of(1, "Field3"), instruction_of(6, "Field3"))); // different types


  REQUIRE(same_dict(instruction_of(2, "Field4"), instruction_of(5, "Field4"))); // type
  REQUIRE(same_dict(instruction_of(3, "Field4"), instruction_of(4, "Field4"))); // global
  REQUIRE(different_dict(instruction_of(2, "Field4"), instruction_of(4, "Field4"))); // type vs global
  REQUIRE(different_dict(instruction_of(1, "Field4"), instruction_of(4, "Field4"))); // template vs global
  REQUIRE(different_dict(instruction_of(1, "Field4"), instruction_of(2, "Field4"))); // template vs type

  REQUIRE(different_dict(instruction_of(1, "Field5"), instruction_of(2, "Field5"))); // different templates
  REQUIRE(same_dict(instruction_of(1, "Field4"), instruction_of(1, "Field8"))); // same key

}

