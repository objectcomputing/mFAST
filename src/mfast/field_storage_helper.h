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
#ifndef FIELD_STORAGE_HELPER_H_D6K0WHUO
#define FIELD_STORAGE_HELPER_H_D6K0WHUO

#include "mfast/field_ref.h"

namespace mfast
{
namespace detail {

class field_storage_helper
{
  public:
    static value_storage_t& storage_of(const field_cref& ref)
    {
      return *const_cast<value_storage_t*>(ref.storage());
    }

    static value_storage_t* storage_ptr_of(const field_cref& ref)
    {
      return const_cast<value_storage_t*>(ref.storage());
    }
};

}
}


#endif /* end of include guard: FIELD_STORAGE_HELPER_H_D6K0WHUO */
