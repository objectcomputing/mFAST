#ifndef CONVERTER_H_YSTETNE8
#define CONVERTER_H_YSTETNE8
#include "mfast_sqlite3_export.h"
#include "field_masks.h"

namespace mfast
{
  class template_instruction;
  namespace SQLite3
  {
    class converter_core;
    class MFAST_SQLITE3_EXPORT converter
    {
    public:
      converter(const char* dbfile, const field_masks& masks);
      ~converter();

      void add_instruction(const mfast::template_instruction*, bool to_create_tables);
      void insert(const aggregate_cref& ref);
    private:
      converter_core* core_;
    };
  } /* SQLite3 */

} /* mfast */


#endif /* end of include guard: CONVERTER_H_YSTETNE8 */
