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
#ifndef OUTPUT_H_6GES63G2
#define OUTPUT_H_6GES63G2

#include "int_ref.h"
#include "string_ref.h"
#include "decimal_ref.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <boost/io/ios_state.hpp>

namespace mfast {
inline std::ostream& operator << (std::ostream& os, const int32_cref& cref)
{
  os << cref.value();
  return os;
}

inline std::ostream& operator << (std::ostream& os, const uint32_cref& cref)
{
  os << cref.value();
  return os;
}

inline std::ostream& operator << (std::ostream& os, const int64_cref& cref)
{
  os << cref.value();
  return os;
}

inline std::ostream& operator << (std::ostream& os, const uint64_cref& cref)
{
  os << cref.value();
  return os;
}

inline std::ostream& operator << (std::ostream& os, const ascii_string_cref& cref)
{
  os << cref.c_str();
  return os;
}

inline std::ostream& operator << (std::ostream& os, const unicode_string_cref& cref)
{
  os << cref.c_str();
  return os;
}

inline std::ostream& operator << (std::ostream& os, const byte_vector_cref& cref)
{
  boost::io::ios_flags_saver  ifs( os );
  for (int i = 0 ; i < cref.size(); ++i){
    os << (int) cref[i];
  }
  return os;
}

inline std::ostream& operator << (std::ostream& os, const decimal_mref& cref)
{
  if (cref.exponent() >=0 ){
    os << cref.mantissa();
    for (int i = 0; i < cref.exponent(); ++i) {
      os << "0";
    }
  }
  else {
    std::stringstream temp;
    temp << cref.mantissa();
    std::string str = temp.str();
    int integer_part_digits = str.size() + cref.exponent();
    if (integer_part_digits > 0) {
      str.insert(integer_part_digits, 1, '.');
      os << str;
    }
    else if (integer_part_digits < 0){
      boost::io::ios_flags_saver  ifs( os );
      os.precision(-integer_part_digits);
      os.setf( std::ios::fixed, std:: ios::floatfield ); // floatfield set to fixed
      os << 0.0L << temp;
    }
    else {
      os << "0." << temp;
    }
  }
  return os;
}
}

#endif /* end of include guard: OUTPUT_H_6GES63G2 */
