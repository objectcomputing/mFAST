#include "error.h"

namespace mfast {
namespace SQLite3 {

sqlite3_error::sqlite3_error(sqlite3 *db, const char *api_name) {
  *this << boost::errinfo_api_function(api_name)
        << sqlite3_errmsg_info(sqlite3_errmsg(db));
}

} /* SQLite3 */

} /* mfast */

namespace boost {
template class error_info<mfast::SQLite3::tag_reason, std::string>;
template class error_info<mfast::SQLite3::tag_sqlite3_errmsg, std::string>;
template class error_info<mfast::SQLite3::tag_field_name, std::string>;
template class error_info<mfast::SQLite3::tag_column, unsigned>;
}
