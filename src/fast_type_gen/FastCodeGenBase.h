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
#ifndef FASTCODEGEN_H_RV7SDOTE
#define FASTCODEGEN_H_RV7SDOTE

#include "../fastxml/FastXMLVisitor.h"
#include <map>
#include <string>
#include <boost/exception/all.hpp>

class file_open_error
  : public virtual boost::exception, public virtual std::exception
{
  public:
    file_open_error()
    {
    }

    file_open_error(const std::string& filename)
    {
      *this << boost::errinfo_file_name(filename) << boost::errinfo_errno(errno);
    }

};




class FastCodeGenBase
  : public FastXMLVisitor
{
protected:
  std::string filebase_;
  std::ofstream out_;
public:
  FastCodeGenBase(const char* filebase, const char* fileext)
    : filebase_(filebase)
    , out_((filebase_+fileext).c_str(), std::ofstream::trunc)
  {
    if (!out_.is_open()) {
      filebase_ += fileext;
      throw file_open_error(filebase_);
    }
  }
};

typedef std::map<std::string, std::string> templates_registry_t;

#endif /* end of include guard: FASTCODEGEN_H_RV7SDOTE */
