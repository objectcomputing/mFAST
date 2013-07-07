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
#ifndef FASTXML2HEADER_H_5QUNM0Q5
#define FASTXML2HEADER_H_5QUNM0Q5

#include "FastCodeGenBase.h"
#include "indented_ostream.h"
#include <boost/algorithm/string.hpp>


class FastXML2Header
  : public FastCodeGenBase
{
  public:
    FastXML2Header(const char* filebase, templates_registry_t& registry);
    /// Visit a document.
    
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif
    
    virtual bool VisitEnter( const XMLDocument& /*doc*/ );
    /// Visit a document.
    virtual bool VisitExit( const XMLDocument& /*doc*/ );
    
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    
    virtual bool  VisitEnterTemplate (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool  VisitExitTemplate (const XMLElement & element, const std::string& name_attr, std::size_t numFields, std::size_t index);
    virtual bool  VisitEnterGroup (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool  VisitExitGroup (const XMLElement & element, const std::string& name_attr, std::size_t numFields, std::size_t index);
    virtual bool  VisitEnterSequence (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool  VisitExitSequence (const XMLElement & element, const std::string& name_attr, std::size_t numFields, std::size_t index);


    virtual bool VisitString (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool VisitInteger (const XMLElement & element, int bits, const std::string& name_attr, std::size_t index);
    virtual bool VisitDecimal (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool VisitByteVector (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool VisitTemplateRef(const XMLElement & element, const std::string& name_attr, std::size_t index);

  private:
    void restore_scope(const std::string& name_attr);

    bool VisitEnterSimpleValue (const XMLElement & element,
                                const char*        cpp_type,
                                const std::string& name_attr,
                                std::size_t        index);

    typedef indented_stringstream ind_stream;

    ind_stream header_cref_;
    ind_stream header_mref_;
    std::stringstream cref_scope_;
    templates_registry_t& registry_;
};


#endif /* end of include guard: FASTXML2HEADER_H_5QUNM0Q5 */
