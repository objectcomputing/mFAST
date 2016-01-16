#pragma once

#include <boost/config.hpp>

#ifndef MFAST_SQLITE3_EXPORT
#ifdef mfast_sqlite3_EXPORTS
/* We are building the shared variant of this library */
#define MFAST_SQLITE3_EXPORT BOOST_SYMBOL_EXPORT
#define MFAST_SQLITE3_BUILD_SHARED_LIBS
#elif defined(MFAST_DYN_LINK)
/* We are using the shared variant of this library */
#define MFAST_SQLITE3_EXPORT BOOST_SYMBOL_IMPORT
#define MFAST_SQLITE3_USE_SHARED_LIBS
#else
/* We are either using or building static libs */
#define MFAST_SQLITE3_EXPORT
#define MAST_SQLITE3_NO_SHARED_LIBS
#endif
#endif

