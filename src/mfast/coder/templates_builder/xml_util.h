#ifndef XML_UTIL_H_N23DJCGA
#define XML_UTIL_H_N23DJCGA

#include "../../../../tinyxml2/tinyxml2.h"
#include "mfast/arena_allocator.h"

namespace mfast
{
namespace coder
{
using namespace tinyxml2;
inline const char* get_optional_attr(const XMLElement & element,
                                     const char*        attr_name,
                                     const char*        default_value)
{
  const XMLAttribute* attr = element.FindAttribute(attr_name);
  if (attr == 0) {
    return default_value;
  }
  return attr->Value();
}

inline const char*
string_dup(const char* str, arena_allocator&  alloc)
{
  if (str == 0 || str[0] == '\x0')
    return "";
  std::size_t len = std::strlen(str);
  char* result = static_cast<char*>( alloc.allocate(len+1) );
  std::strcpy(result, str);
  return result;
}

}   /* coder */

} /* mfast */


#endif /* end of include guard: XML_UTIL_H_N23DJCGA */
