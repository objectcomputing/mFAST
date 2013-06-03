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
#ifndef FASTXML2INLINE_H_YZITSJ3U
#define FASTXML2INLINE_H_YZITSJ3U

#include <sstream>
#include <boost/algorithm/string.hpp>

#include "FastCodeGenBase.h"

class FastXML2Inline
  : public FastCodeGenBase
{
  private:

    std::stringstream cref_scope_;
    std::stringstream mref_scope_;
    templates_registry_t& registry_;
  public:
    FastXML2Inline(const char* filebase, templates_registry_t& registry);

    void restore_scope(const std::string& name_attr);

    virtual bool  VisitEnterTemplate (const XMLElement & element, const std::string& name_att, std::size_t indexr);
    virtual bool  VisitExitTemplate (const XMLElement & element, const std::string& name_attr, std::size_t numFields, std::size_t index);
    virtual bool  VisitEnterGroup (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool  VisitExitGroup (const XMLElement & element, const std::string& name_attr, std::size_t numFields, std::size_t index);
    virtual bool  VisitEnterSequence (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool  VisitExitSequence (const XMLElement & element, const std::string& name_attr, std::size_t numFields, std::size_t index);

    bool VisitEnterSimpleValue (const XMLElement & element, const char* cpp_type, const std::string& name_attr, std::size_t index);

    virtual bool VisitString (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool VisitInteger (const XMLElement & element, int bits, const std::string& name_attr, std::size_t index);
    virtual bool VisitDecimal (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool VisitByteVector (const XMLElement & element, const std::string& name_attr, std::size_t index);
    virtual bool VisitTemplateRef(const XMLElement & element, const std::string& name_attr, std::size_t index);
};


#endif /* end of include guard: FASTXML2INLINE_H_YZITSJ3U */
