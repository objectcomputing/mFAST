#pragma once

#include "mfast_sqlite3_export.h"
#include "../mfast_export.h"

#include <sqlite3.h>
#include <boost/exception/errinfo_api_function.hpp>
#include <boost/exception/errinfo_at_line.hpp>
#include <boost/exception/errinfo_errno.hpp>
#include <boost/exception/errinfo_file_handle.hpp>
#include <boost/exception/errinfo_file_name.hpp>
#include <boost/exception/errinfo_file_open_mode.hpp>
#include <boost/exception/info.hpp>
#include <boost/throw_exception.hpp>

namespace mfast {
namespace SQLite3 {
struct MFAST_SQLITE3_EXPORT error : virtual std::exception,
                                    virtual boost::exception {};

struct tag_reason;
struct tag_sqlite3_errmsg;
struct tag_field_name;
struct tag_column;

} /* SQLite3 */

} /* mfast */

#if !defined(BOOST_NO_CXX11_EXTERN_TEMPLATE)
namespace boost {
// For Clang, we must use extern template and explicit template instantiation;
//     otherwise, we will have duplicated definition link error when building
//     shared library.
// For GCC, we must nest the explicit instantiation statement inside their
// original namespace;
//     otherwise, the code won't compile.
extern template class error_info<mfast::SQLite3::tag_reason, std::string>;
extern template class error_info<mfast::SQLite3::tag_sqlite3_errmsg,
                                 std::string>;
extern template class error_info<mfast::SQLite3::tag_field_name, std::string>;
extern template class error_info<mfast::SQLite3::tag_column, unsigned>;
}
#endif

namespace mfast {
namespace SQLite3 {
typedef boost::error_info<tag_reason, std::string> reason_info;
typedef boost::error_info<tag_sqlite3_errmsg, std::string> sqlite3_errmsg_info;
typedef boost::error_info<tag_field_name, std::string> field_name_info;
typedef boost::error_info<tag_column, unsigned> table_column_info;

struct MFAST_SQLITE3_EXPORT sqlite3_error : error {
  sqlite3_error(sqlite3 *db, const char *api_name);
};

} /* SQLite3 */

} /* mfast */
