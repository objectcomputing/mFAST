#pragma once

#include "field_masks.h"

namespace mfast {
namespace SQLite3 {
typedef std::map<unsigned, std::string> bindings_t;
struct template_info {
  template_info()
      : insert_stmt_(nullptr), find_key_stmt_(nullptr), primary_key_index_(-1) {}
  sqlite3_stmt *insert_stmt_;
  sqlite3_stmt *find_key_stmt_;
  int primary_key_index_;
  std::string insert_text_, find_key_text_;
  mutable bindings_t binding_;
};

class converter_core : public field_masks {
public:
  converter_core(const char *db_file, const field_masks &);

  ~converter_core();

  void add(const template_instruction *inst, bool to_create_tables);

  const template_info &info(const group_field_instruction *inst) const;

  field_cref primary_key_of(const aggregate_cref &ref) const;

  int exec_stmt(const template_info &info) const;

  void bind(const template_info &info, unsigned column, int64_t value) const;

  template <typename T>
  void bind(const template_info &info, unsigned column,
            const int_cref<T> &ref) const {
    bind(info, column, ref.value());
  }

  void bind(const template_info &info, unsigned column,
            const enum_cref &ref) const {
    bind(info, column, ref.value());
  }

  void bind(const template_info &info, unsigned column,
            const decimal_cref &ref) const;
  void bind(const template_info &info, unsigned column, const char *str) const;

  template <typename T>
  void bind(const template_info &info, unsigned column,
            const string_cref<T> &ref) const {
    bind(info, column, ref.c_str());
  }

  void bind(const template_info &info, unsigned column,
            const byte_vector_cref &ref) const;

  void bind_null(const template_info &info, unsigned column) const;

  void bind(const template_info &info, unsigned column,
            const field_cref &ref) const;

private:
  sqlite3 *db_;
  typedef std::map<unsigned, template_info> template_infos_t;
  template_infos_t infos_;
};

} /* SQLite3 */
} /* mfast */
