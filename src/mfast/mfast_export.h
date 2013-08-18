
#ifndef MFAST_EXPORT_H
#define MFAST_EXPORT_H

#include <boost/config.hpp>

#ifdef MFAST_STATIC_DEFINE
#  define MFAST_EXPORT
#  define MFAST_NO_EXPORT
#else
#  ifndef MFAST_EXPORT
#    ifdef mfast_EXPORTS
        /* We are building this library */
#      define MFAST_EXPORT BOOST_SYMBOL_EXPORT 
#    else
        /* We are using this library */
#      define MFAST_EXPORT BOOST_SYMBOL_IMPORT 
#    endif
#  endif

#  ifndef MFAST_NO_EXPORT
#    define MFAST_NO_EXPORT 
#  endif
#endif

#if !defined(BOOST_NO_CXX11_EXTERN_TEMPLATE)
#  if  (defined(__GNUC__) && (__GNUC__ >= 3)) && (!defined(__clang__))
// g++ extern template class does not work when the template class has static data members, need to use inline stead.
#    define MFAST_EXTERN_TEMPLATE inline
#  else
#    define MFAST_EXTERN_TEMPLATE extern
#  endif
#endif

#endif
