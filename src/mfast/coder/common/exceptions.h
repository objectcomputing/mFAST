#ifndef EXCEPTIONS_H_ITUZXSUY
#define EXCEPTIONS_H_ITUZXSUY

#include "mfast/exceptions.h"


namespace mfast
{
  namespace coder
  {
    struct tag_template_id;
    struct tag_referenced_by;
    struct tag_template_name;

    typedef boost::error_info<tag_template_id,unsigned> template_id_info;
    typedef boost::error_info<tag_referenced_by,std::string> referenced_by_info;
    typedef boost::error_info<tag_template_name,std::string> template_name_info;

    class template_not_found_error
      : public fast_dynamic_error
    {
    public:
      template_not_found_error(const char* template_name, const char* referenced_by)
        : fast_dynamic_error("D8")
      {
        *this << template_name_info(template_name) << referenced_by_info(referenced_by);
      }

    };


    class duplicate_template_id_error
      : public fast_static_error
    {
    public:
      duplicate_template_id_error(unsigned tid)
      {
        *this << template_id_info(tid);
      }

    };
  } /* coder */

} /* mfast */


#endif /* end of include guard: EXCEPTIONS_H_ITUZXSUY */
