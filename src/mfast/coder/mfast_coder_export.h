
#pragma once
#include <boost/config.hpp>

#ifndef MFAST_CODER_EXPORT
#ifdef mfast_coder_EXPORTS
/* We are building the shared variant of this library */
#define MFAST_CODER_EXPORT BOOST_SYMBOL_EXPORT
#define MFAST_CODER_BUILD_SHARED_LIBS
#elif defined(MFAST_DYN_LINK)
/* We are using the shared variant of this library */
#define MFAST_CODER_EXPORT BOOST_SYMBOL_IMPORT
#define MFAST_CODER_USE_SHARED_LIBS
#else
/* We are either using or building static libs */
#define MFAST_CODER_EXPORT
#define MAST_CODER_NO_SHARED_LIBS
#endif
#endif
