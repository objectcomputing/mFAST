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
#include "mfast/sequence_ref.h"
#include "mfast/allocator.h"
#include <limits>

namespace mfast {
  namespace detail {

    const value_storage default_sequence_storage;

    void sequence_mref_helper::reserve(const sequence_field_instruction* instruction,
                                       value_storage*                    storage,
                                       allocator*                        alloc,
                                       std::size_t                       n)
    {
      assert(n < std::numeric_limits<uint32_t>::max());
      std::size_t element_size = instruction->group_content_byte_count ();

      std::size_t reserve_size = n*element_size;

      if ( reserve_size > storage->of_array.capacity_in_bytes_)
      {

        std::size_t new_capacity =
          alloc->reallocate (storage->of_array.content_,
                             storage->of_array.capacity_in_bytes_,
                             reserve_size);

        std::size_t old_num_elements = storage->of_array.capacity_in_bytes_/element_size;
        std::size_t new_num_elements = new_capacity/element_size;

        instruction->construct_sequence_elements (*storage,
                                                  old_num_elements,
                                                  new_num_elements - old_num_elements,
                                                  alloc);
        // In the above 3rd parameter, it is incorrect to use
        // (new_capacity-storage->of_array.capacity_in_bytes_)/element_size.
        // It would cause differet value when either storage->of_array.capacity_in_bytes_
        // or new_capacity is not the multiple of element_size.

        storage->of_array.capacity_in_bytes_ = static_cast<uint32_t>(new_capacity);
      }
    }

  }
}
