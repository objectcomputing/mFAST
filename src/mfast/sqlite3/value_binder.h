#pragma once

#include <sqlite3.h>
#include "error.h"
namespace mfast {
namespace SQLite3 {
class value_binder {
protected:
  const converter_core &core_;
  const template_info &info_;
  unsigned current_column_;

  void bind_int(int64_t value) { core_.bind(info_, current_column_++, value); }
  void bind_null() { core_.bind_null(info_, current_column_++); }

public:
  value_binder(const converter_core &core, const template_info &info,
               unsigned current_column)
      : core_(core), info_(info), current_column_(current_column) {}

  template <typename CREF> void bind(const CREF &ref) {
    try {
      core_.bind(info_, current_column_, ref);
      current_column_++;
    } catch (boost::exception &ex) {
      ex << table_column_info(current_column_) << field_name_info(ref.name());
    }
  }
};

} /* SQLite3 */

} /* mfast */
