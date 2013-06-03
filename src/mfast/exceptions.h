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
#ifndef EXCEPTIONS_H_87B9JUIK
#define EXCEPTIONS_H_87B9JUIK

#include <boost/exception/all.hpp>

namespace mfast
{



struct tag_error_code;
typedef boost::error_info<tag_error_code,std::string> fast_error_info;

class fast_error
  : public virtual boost::exception, public virtual std::exception
{
  public:
    fast_error()
    {
    }

    fast_error(const char* error_code)
    {
      *this << fast_error_info(error_code);
    }

};


class fast_static_error
  : public fast_error
{
public:
  fast_static_error()
  {
  };
  fast_static_error(const char* error_code)
    : fast_error(error_code)
  {
  }

};

class fast_dynamic_error
  : public fast_error
{
  public:
    fast_dynamic_error(const char* error_code)
    : fast_error(error_code)
  {
  }

};

class fast_reportable_error
  : public fast_error
{
public:
  fast_reportable_error(const char* error_code)
    : fast_error(error_code)
  {
  }
};


struct tag_template_id;
typedef boost::error_info<tag_template_id,unsigned> template_id_info;
struct tag_template_name;
typedef boost::error_info<tag_template_name,std::string> template_name_info;
struct tag_referenced_by;
typedef boost::error_info<tag_referenced_by,std::string> referenced_by_info;
struct tag_reason;
typedef boost::error_info<tag_referenced_by,std::string> reason_info;

class duplicate_template_id_error
  : public fast_static_error
{
  public:
    duplicate_template_id_error(unsigned tid)
    {
      *this << template_id_info(tid);
    }

};

class template_not_found_error
  : public fast_dynamic_error
{
  public:
    template_not_found_error(const char* template_name, const char* referenced_by)
      : fast_dynamic_error("D8")
    {
      *this << template_name_info(template_name) << referenced_by_info(referenced_by);
    }

};

}


#endif /* end of include guard: EXCEPTIONS_H_87B9JUIK */
