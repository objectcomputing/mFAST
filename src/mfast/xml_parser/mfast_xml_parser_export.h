#pragma once

#include <boost/config.hpp>

#ifndef MFAST_XML_PARSER_EXPORT
#ifdef mfast_xml_parser_EXPORTS
/* We are building the shared variant of this library */
#define MFAST_XML_PARSER_EXPORT BOOST_SYMBOL_EXPORT
#define MFAST_XML_PARSER_BUILD_SHARED_LIBS
#elif defined(MFAST_DYN_LINK)
/* We are using the shared variant of this library */
#define MFAST_XML_PARSER_EXPORT BOOST_SYMBOL_IMPORT
#define MFAST_XML_PARSER_USE_SHARED_LIBS
#else
/* We are either using or building static libs */
#define MFAST_XML_PARSER_EXPORT
#define MAST_XML_PARSER_NO_SHARED_LIBS
#endif
#endif
