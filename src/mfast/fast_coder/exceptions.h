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
#include "mfast/mfast_export.h"

#ifdef BOOST_MSVC 
# pragma warning(push) 
# pragma warning(disable : 4275) 
#endif 

namespace mfast
{

// we should always export exception classes; otherwise, the vtable won't
// be available for application when they are in shared libraries.

class MFAST_EXPORT fast_error
  : public virtual boost::exception, public virtual std::exception
{
  public:
    fast_error()
    {
    }

    fast_error(const char* error_code);

};


class MFAST_EXPORT fast_static_error
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

class MFAST_EXPORT fast_dynamic_error
  : public fast_error
{
  public:
    fast_dynamic_error(const char* error_code)
    : fast_error(error_code)
  {
  }

};

class MFAST_EXPORT fast_reportable_error
  : public fast_error
{
public:
  fast_reportable_error(const char* error_code)
    : fast_error(error_code)
  {
  }
};

struct tag_referenced_by;
struct tag_template_id;
}

namespace  boost {
  // For Clang, we must use extern template and explicit template instantiation; 
  //     otherwise, we will have duplicated definition link error when building shared library.
  // For GCC, we must nest the explicit instantiation statement inside their original namespace;
  //     otherwise, the code won't compile.
extern template class error_info<mfast::tag_referenced_by,std::string>;
extern template class error_info<mfast::tag_template_id,unsigned>;
}

namespace mfast {
typedef boost::error_info<tag_referenced_by,std::string> referenced_by_info; 
typedef boost::error_info<tag_template_id,unsigned> template_id_info;
}


#ifdef BOOST_MSVC 
#pragma warning(pop) 
#endif 

#endif /* end of include guard: EXCEPTIONS_H_87B9JUIK */
