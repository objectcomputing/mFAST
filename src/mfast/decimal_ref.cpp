// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#include <cmath>
#include <cfloat>

#include <iostream>

#include <boost/multiprecision/number.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

template class boost::multiprecision::backends::cpp_dec_float<18>;

#if BOOST_VERSION >= 106200

#include <boost/functional/hash.hpp>

#if defined(_MSC_VER) 
#pragma warning(disable: 4667)  //  no function template defined that matches forced instantiation
#else
// This doesn't work in MSVC, but apparently it isn't needed.
template void boost::hash_combine<boost::multiprecision::backends::cpp_dec_float<18u, int, void>::enum_fpclass_type>(unsigned long&, boost::multiprecision::backends::cpp_dec_float<18u, int, void>::enum_fpclass_type const&);
#endif
template void boost::hash_combine<bool>(unsigned long&, bool const&);
template void boost::hash_combine<int>(unsigned long&, int const&);
template void boost::hash_combine<unsigned int>(unsigned long&, unsigned int const&);
#endif