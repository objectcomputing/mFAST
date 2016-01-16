#include <math.h>
#include "converter_core.h"
#include "error.h"
#include "tables_creator.h"
#include <boost/io/ios_state.hpp>
#include <mfast/output.h>
#include <boost/lexical_cast.hpp>
#include "field_instruction_visitor_ex.h"

namespace mfast {
namespace SQLite3 {

std::string format_binding(const bindings_t &binding) {
  std::stringstream strm;
  for (auto &&entry : binding) {
    strm << "?" << entry.first << "=" << entry.second << "\t";
  }
  return strm.str();
}

struct tag_statement;
typedef boost::error_info<tag_statement, std::string> statement_info;

struct tag_binding;
struct binding_info : public boost::error_info<tag_binding, std::string> {
  binding_info(const std::map<unsigned, std::string> &binding)
      : boost::error_info<tag_binding, std::string>(format_binding(binding)) {}
};

converter_core::converter_core(const char *dbfile, const field_masks &mask)
    : field_masks(mask) {
  if (sqlite3_open_v2(dbfile, &db_, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,
                      nullptr)) {
    BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_open")
                          << boost::errinfo_file_name(dbfile));
  }
}

converter_core::~converter_core() { sqlite3_close(db_); }

void converter_core::add(const template_instruction *inst,
                         bool to_create_tables) {
  tables_creator::insert_map_t the_map;
  tables_creator creator(the_map, *this);
  creator.visit(inst, nullptr);
#if 0
      std::cout << "create statement : " << "\n" << creator.create_statements() << "\n";
#endif
  if (to_create_tables) {
    if (sqlite3_exec(db_, creator.create_statements().c_str(), nullptr, nullptr, nullptr) !=
        SQLITE_OK) {
      BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_exec"));
    }
  }

  for (auto &&v : the_map) {
#if 0
        std::cout << "preparing statement : " << v.second.insert_item_stmt << "\n";
#endif
    template_info &info = infos_[v.first];
    if (sqlite3_prepare_v2(db_, v.second.insert_item_stmt.c_str(),
                           v.second.insert_item_stmt.size(), &info.insert_stmt_,
                           nullptr) != SQLITE_OK) {
      BOOST_THROW_EXCEPTION(
          sqlite3_error(db_, "sqlite3_prepare_v2")
          << statement_info(v.second.insert_item_stmt.c_str()));
    }

    if (v.second.find_key_stmt.size()) {
#if 0
          std::cout << "preparing statement : " << v.second.find_key_stmt << "\n";
#endif
      if (sqlite3_prepare_v2(db_, v.second.find_key_stmt.c_str(),
                             v.second.find_key_stmt.size(),
                             &info.find_key_stmt_, nullptr) != SQLITE_OK) {
        BOOST_THROW_EXCEPTION(
            sqlite3_error(db_, "sqlite3_prepare_v2")
            << statement_info(v.second.find_key_stmt.c_str()));
      }
    }
    info.primary_key_index_ = v.second.primary_key_index;
    info.insert_text_ = v.second.insert_item_stmt;
    info.find_key_text_ = v.second.find_key_stmt;
  }
}

struct unsupported_group_error : error {};

const template_info &
converter_core::info(const group_field_instruction *inst) const {
  unsigned id = (inst->ref_instruction() != nullptr) ? inst->ref_instruction()->id()
                                               : inst->id();
  template_infos_t::const_iterator itr = infos_.find(id);
  if (itr == infos_.end())
    BOOST_THROW_EXCEPTION(unsupported_group_error()
                          << field_name_info(inst->name()));

  return itr->second;
}

field_cref converter_core::primary_key_of(const aggregate_cref &ref) const {
  int primary_key_index = info(ref.instruction()).primary_key_index_;
  if (primary_key_index == -1)
    return field_cref();
  return ref[primary_key_index];
}

int converter_core::exec_stmt(const template_info &info) const {
#if 0
      std::cout << "executing " << info.insert_text_ << "\n"
                << "with binding : " << format_binding(info.binding_) << "\n";
#endif
  if (sqlite3_step(info.insert_stmt_) == SQLITE_DONE) {
    sqlite3_reset(info.insert_stmt_);
    int rowid = sqlite3_last_insert_rowid(db_);
    return rowid;
  }

  std::string saved_reason = sqlite3_errmsg(db_);

  if (info.find_key_stmt_) {
#if 0
      std::cout << "executing " << info.find_key_text_ << "\n"
                << "with binding : " << format_binding(info.binding_) << "\n";
#endif
    int ret = sqlite3_step(info.find_key_stmt_);
    if (ret == SQLITE_ROW) {
      int result = sqlite3_column_int(info.find_key_stmt_, 0);
#if 0
      std::cout << "rowid found at " << result << "\n";
#endif
      sqlite3_reset(info.insert_stmt_);
      sqlite3_reset(info.find_key_stmt_);
      return result;
    }
  }
  BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_step")
                        << statement_info(info.insert_text_)
                        << binding_info(info.binding_)
                        << reason_info(saved_reason));
}

void converter_core::bind(const template_info &info, unsigned column,
                          int64_t value) const {
  info.binding_[column] = boost::lexical_cast<std::string>(value);

  if (sqlite3_bind_int(info.insert_stmt_, column, value) != SQLITE_OK)
    BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_int"));

  if (info.find_key_stmt_)
    if (sqlite3_bind_int(info.find_key_stmt_, column, value) != SQLITE_OK)
      BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_int"));
}

void converter_core::bind(const template_info &info, unsigned column,
                          const decimal_cref &ref) const {
  if (has_constant_exponent(ref))
    bind(info, column, ref.mantissa());
  else {
    double value = pow(10, ref.exponent()) * ref.mantissa();

    info.binding_[column] = boost::lexical_cast<std::string>(value);
    if (sqlite3_bind_double(info.insert_stmt_, column, value) != SQLITE_OK)
      BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_real"));
    if (info.find_key_stmt_)
      if (sqlite3_bind_double(info.find_key_stmt_, column, value) != SQLITE_OK)
        BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_real"));
  }
}

void converter_core::bind(const template_info &info, unsigned column,
                          const char *str) const {
  info.binding_[column] = str;

  if (sqlite3_bind_text(info.insert_stmt_, column, str, -1, SQLITE_STATIC) !=
      SQLITE_OK)
    BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_text"));
  if (info.find_key_stmt_)
    if (sqlite3_bind_text(info.find_key_stmt_, column, str, -1,
                          SQLITE_STATIC) != SQLITE_OK)
      BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_text"));
}

void converter_core::bind(const template_info &info, unsigned column,
                          const byte_vector_cref &ref) const {
  std::stringstream strm;

  strm << std::hex << std::setfill('0') << std::setw(2);

  for (std::size_t i = 0; i < ref.size(); ++i) {
    strm << (0xFF & (int)ref[i]);
  }

  strm << "\",";
  info.binding_[column] = strm.str();

  if (sqlite3_bind_blob(info.insert_stmt_, column, ref.data(), ref.size(),
                        SQLITE_STATIC) != SQLITE_OK)
    BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_blob"));
  if (info.find_key_stmt_)
    if (sqlite3_bind_blob(info.find_key_stmt_, column, ref.data(), ref.size(),
                          SQLITE_STATIC) != SQLITE_OK)
      BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_blob"));
}

void converter_core::bind_null(const template_info &info,
                               unsigned column) const {
  info.binding_[column] = "NULL";

  if (sqlite3_bind_null(info.insert_stmt_, column) != SQLITE_OK)
    BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_null"));
  if (info.find_key_stmt_)
    if (sqlite3_bind_null(info.find_key_stmt_, column) != SQLITE_OK)
      BOOST_THROW_EXCEPTION(sqlite3_error(db_, "sqlite3_bind_null"));
}

class field_cref_binder : public field_instruction_visitor_ex {
  const converter_core &core_;
  const template_info &info_;
  unsigned column_;

public:
  field_cref_binder(const converter_core &core, const template_info &info,
                    unsigned column)
      : core_(core), info_(info), column_(column) {}

  void visit(const integer_field_instruction_base *, void *storage) {
    core_.bind(info_, column_,
               static_cast<value_storage *>(storage)->get<int64_t>());
  }

  void visit(const ascii_field_instruction *, void *storage) {
    core_.bind(info_, column_,
               static_cast<value_storage *>(storage)->get<const char *>());
  }

  void visit(const byte_vector_field_instruction *inst, void *storage) {
    byte_vector_cref ref(static_cast<value_storage *>(storage), inst);
    core_.bind(info_, column_, ref);
  }

  void visit(const vector_field_instruction_base *, void *) {
    BOOST_THROW_EXCEPTION(
        std::runtime_error("only primitive types are supported"));
  }

  void visit(const templateref_instruction *, void *) {
    BOOST_THROW_EXCEPTION(
        std::runtime_error("only primitive types are supported"));
  }

  void visit(const group_field_instruction *, void *) {
    BOOST_THROW_EXCEPTION(
        std::runtime_error("only primitive types are supported"));
  }

  void visit(const sequence_field_instruction *, void *) {
    BOOST_THROW_EXCEPTION(
        std::runtime_error("only primitive types are supported"));
  }

  void visit(const template_instruction *, void *) {
    BOOST_THROW_EXCEPTION(
        std::runtime_error("only primitive types are supported"));
  }
};

void converter_core::bind(const template_info &info, unsigned column,
                          const field_cref &ref) const {
  field_cref_binder binder(*this, info, column);
  ref.instruction()->accept(
      binder,
      const_cast<value_storage *>(field_cref_core_access::storage_of(ref)));
}

} /* SQLite3 */

} /* mfast */
