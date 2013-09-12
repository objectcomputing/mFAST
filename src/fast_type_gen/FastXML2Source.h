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
#ifndef FASTXML2SOURCE_H_DKDUNNEW
#define FASTXML2SOURCE_H_DKDUNNEW

#include <sstream>
#include <set>
#include "FastCodeGenBase.h"

class FastXML2Source
  : public FastCodeGenBase
{


  public:
    FastXML2Source(const char* filebase, templates_registry_t& registry);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    /// Visit a document.
    virtual bool VisitEnter( const XMLDocument& /*doc*/ );
    /// Visit a document.
    virtual bool VisitExit( const XMLDocument& /*doc*/ );

#ifdef __clang__
#pragma clang diagnostic pop
#endif

    void add_to_instruction_list(const std::string & name_attr);

    virtual bool  VisitExitTemplates (const XMLElement & element,
                                      std::size_t        numFields);

    virtual bool  VisitEnterTemplate (const XMLElement & element,
                                      const std::string& name_attr,
                                      std::size_t        index);

    virtual bool  VisitExitTemplate (const XMLElement & element,
                                     const std::string& name_attr,
                                     std::size_t        numFields,
                                     std::size_t        index);

    virtual bool  VisitEnterGroup (const XMLElement & element,
                                   const std::string& name_attr,
                                   std::size_t        index);
    virtual bool  VisitExitGroup (const XMLElement & element,
                                  const std::string& name_attr,
                                  std::size_t        numFields,
                                  std::size_t        index);
    virtual bool  VisitEnterSequence (const XMLElement & element,
                                      const std::string& name_attr,
                                      std::size_t        index);
    virtual bool  VisitExitSequence (const XMLElement & element,
                                     const std::string& name_attr,
                                     std::size_t        numFields,
                                     std::size_t        index);

    virtual bool VisitString (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool VisitInteger (const XMLElement & element, int bits, const std::string& name_attr, std::size_t index);
    virtual bool VisitDecimal (const XMLElement & element,
                               const std::string& name_attr,
                               std::size_t        index);
    virtual bool VisitByteVector (const XMLElement & element,
                                  const std::string& name_attr,
                                  std::size_t        index);

    virtual bool VisitTemplateRef(const XMLElement & element, const std::string& name_attr, std::size_t index);

  private:

    bool output_typeref(const XMLElement & element);
    const XMLElement* fieldOpElement(const XMLElement & element);
    bool get_field_attributes(const XMLElement & element,
                              const std::string& name_attr,
                              std::string&       fieldOpName,
                              std::string&       opContext,
                              std::string&       initialValue);
    void output_subinstructions(const std::string name_attr);
    void restore_scope(const std::string& name_attr);

    std::string get_subinstructions(const XMLElement & element,
                                    const std::string& name_attr,
                                    std::size_t        numFields);
    std::stringstream cref_scope_;
    std::vector<std::string> subinstructions_list_;
    std::stringstream template_instructions_;
    templates_registry_t& registry_;
    std::string templates_ns_;
};

#endif /* end of include guard: FASTXML2SOURCE_H_DKDUNNEW */
