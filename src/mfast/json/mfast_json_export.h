
#ifndef MFAST_JSON_EXPORT_H
#define MFAST_JSON_EXPORT_H

#include <boost/config.hpp>


#ifndef MFAST_JSON_EXPORT
#  ifdef mfast_json_EXPORTS
      /* We are building the shared variant of this library */
#    define MFAST_JSON_EXPORT BOOST_SYMBOL_EXPORT
#    define MFAST_JSON_BUILD_SHARED_LIBS
#  elif defined(MFAST_DYN_LINK)
     /* We are using the shared variant of this library */
#    define MFAST_JSON_EXPORT BOOST_SYMBOL_IMPORT
#    define MFAST_JSON_USE_SHARED_LIBS
#  else
    /* We are either using or building static libs */
#    define MFAST_JSON_EXPORT
#    define MAST_JSON_NO_SHARED_LIBS
#  endif
#endif


#endif // MFAST_JSON_EXPORT_H
