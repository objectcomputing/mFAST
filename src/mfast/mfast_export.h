// Copyright (c) 2016, Huang-Ming Huang,  Object Computing, Inc.
// All rights reserved.
//
// This file is part of mFAST.
// See the file license.txt for licensing information.
#pragma once

#include <boost/config.hpp>

#ifndef MFAST_EXPORT
#ifdef mfast_EXPORTS

/* We are building the shared variant of this library */
#define MFAST_EXPORT BOOST_SYMBOL_EXPORT
#define MFAST_BUILD_SHARED_LIBS
#elif defined(MFAST_DYN_LINK)

/* We are using the shared variant of this library */
#define MFAST_EXPORT BOOST_SYMBOL_IMPORT
#define MFAST_USE_SHARED_LIBS
#else

/* We are either using or building static libs */
#define MFAST_EXPORT
#define MAST_NO_SHARED_LIBS
#endif
#endif

#if !defined(BOOST_NO_CXX11_EXTERN_TEMPLATE)
#if (defined(__GNUC__) && (__GNUC__ >= 3)) && (!defined(__clang__))
// g++ extern template class does not work when the template class has static
// data members, need to
// use inline stead.
#define MFAST_EXTERN_TEMPLATE inline
#else
#define MFAST_EXTERN_TEMPLATE extern
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1700)
#pragma warning(disable : 4355)
#if !defined(BOOST_NO_CXX11_EXTERN_TEMPLATE)
#define BOOST_NO_CXX11_EXTERN_TEMPLATE
#endif
#endif
