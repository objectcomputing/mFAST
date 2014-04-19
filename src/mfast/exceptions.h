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
#ifndef EXCEPTIONS_H_87B9JUIK
#define EXCEPTIONS_H_87B9JUIK
#include "mfast_export.h"
#include <boost/exception/all.hpp>

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
}


#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#endif /* end of include guard: EXCEPTIONS_H_87B9JUIK */
