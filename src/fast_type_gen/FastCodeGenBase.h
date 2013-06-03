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

class file_open_error
  : public std::exception
{
  char filename_[128];

  public:
    file_open_error(const std::string& filename)
    {
      std::strncpy(filename_, filename.c_str(), 127);
      filename_[127] = 0;
    }

    const char* what() const throw()
    {
      static char buffer[256];
      std::snprintf(buffer, 256, "Cannot open file: %s\n", filename_);
      return buffer;
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
