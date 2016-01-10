#include "tables_creator.h"
#include <boost/exception/all.hpp>

namespace mfast {
namespace SQLite3 {

std::string construct_insert_statement(const char *table_name,
                                       unsigned num_columns) {
  std::stringstream strm;
  strm << "INSERT INTO " << table_name << " VALUES (";
  for (unsigned i = 1; i <= num_columns; ++i) {
    strm << "?" << i << ",";
  }

  strm.seekp(-1, std::ios_base::cur);
  strm << ");";
  return strm.str();
}

std::string construct_find_key_statement(const char *table_name,
                                         std::vector<const char *> parameters) {
  std::stringstream strm;
  strm << "SELECT rowid from " << table_name << " WHERE (";
  for (unsigned i = 0; i < parameters.size(); ++i) {
    strm << parameters[i] << "=?" << i + 1 << " AND ";
  }

  strm.seekp(-5, std::ios_base::cur);
  strm << ");  \n";
  return strm.str();
}

tables_creator::tables_creator(insert_map_t &insert_statements,
                               const field_masks &masks, bool find_key_needed)
    : insert_statements_(insert_statements), masks_(masks),
      primary_key_index_(-1), num_columns_(0),
      find_key_needed_(find_key_needed) {}

tables_creator::tables_creator(insert_map_t &insert_statements,
                               const field_masks &masks,
                               const char *foreign_key_table,
                               const char *foreign_key_name,
                               const char *foreign_key_type)
    : insert_statements_(insert_statements), masks_(masks),
      primary_key_index_(-1), num_columns_(0), find_key_needed_(false) {
  assert(foreign_key_table);
  add_foreign_key(foreign_key_expression_, foreign_key_table, foreign_key_name,
                  foreign_key_type);
}

const char *tables_creator::table_name() const { return table_name_.c_str(); }

const char *tables_creator::primary_key_name() const {
  return primary_key_name_.c_str();
}

const char *tables_creator::primary_key_type() const {
  return primary_key_type_.c_str();
}

const char *tables_creator::extra(const field_instruction *inst,
                                  const char *type, void *pIndex) {
  if (masks_.is_primary_key(inst)) {
    primary_key_name_ = inst->name();
    primary_key_type_ = type;
    primary_key_index_ =
        (pIndex == nullptr) ? 0 : *static_cast<std::size_t *>(pIndex);
    return " PRIMARY KEY,";
  }
  return ",";
}

void tables_creator::add_foreign_key(std::ostream &os, const char *tablename,
                                     const char *key_name,
                                     const char *key_type) {
  os << "  " << tablename << key_name << " " << key_type << " REFERENCES "
     << tablename << "(" << key_name << "),\n";
}

void tables_creator::traverse_subinstructions(
    const group_field_instruction *inst) {
  std::size_t i = 0;
  for (auto subinst : inst->subinstructions()) {
    if (!masks_.to_skip(subinst))
      subinst->accept(*this, &i);
    ++i;
  }
}

void tables_creator::visit(const int32_field_instruction *inst, void *pIndex) {
  create_current_ << "  " << inst->name() << " INT"
                  << extra(inst, "INT", pIndex) << "\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const uint32_field_instruction *inst, void *pIndex) {
  create_current_ << "  " << inst->name() << " INT"
                  << extra(inst, "INT", pIndex) << "\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const int64_field_instruction *inst, void *pIndex) {
  create_current_ << "  " << inst->name() << " INT"
                  << extra(inst, "INT", pIndex) << "\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const uint64_field_instruction *inst, void *pIndex) {
  create_current_ << "  " << inst->name() << " INT"
                  << extra(inst, "INT", pIndex) << "\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const decimal_field_instruction *inst, void *) {
  if (masks_.has_constant_exponent(inst))
    create_current_ << "  " << inst->name() << " INT,\n";
  else
    create_current_ << "  " << inst->name() << " REAL,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const ascii_field_instruction *inst, void *) {
  create_current_ << "  " << inst->name() << " TEXT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const unicode_field_instruction *inst, void *) {
  create_current_ << "  " << inst->name() << " TEXT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const byte_vector_field_instruction *inst,
                           void *pIndex) {
  create_current_ << "  " << inst->name() << " BLOB"
                  << extra(inst, "BLOB", pIndex) << "\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const group_field_instruction *inst, void *) {
  if (inst->ref_instruction() == nullptr) {
    traverse_subinstructions(inst);
  } else {
    const template_instruction *ref =
        dynamic_cast<const template_instruction *>(inst->ref_instruction());
    if (ref) {
      if (this->primary_key_name_.empty())
        BOOST_THROW_EXCEPTION(
            std::runtime_error("Current implementation only allows "
                               "primary key field appears before any "
                               "group field in a template."));

      tables_creator nested_builder(
          insert_statements_, masks_, table_name_.c_str(),
          primary_key_name_.c_str(), primary_key_type_.c_str());
      nested_builder.visit(ref, nullptr);
      create_postfix_ << nested_builder.create_statements();
    } else {
      std::stringstream err_msg;
      err_msg << "Not supported : " << inst->ref_instruction()->name();
      BOOST_THROW_EXCEPTION(std::runtime_error(err_msg.str()));
    }
  }
}

void tables_creator::visit(const sequence_field_instruction *inst, void *) {
  const template_instruction *ref_instruction =
      dynamic_cast<const template_instruction *>(inst->ref_instruction());

  const template_instruction *element_instruction =
      dynamic_cast<const template_instruction *>(inst->element_instruction());

  bool is_ordered = masks_.is_ordered(inst);

  if (ref_instruction == nullptr && element_instruction == nullptr) {
    traverse_subinstructions(inst);
    if (is_ordered) {
      create_current_ << "  ordering INT,\n";
      num_columns_++;
    }
    return;
  }

  if (this->primary_key_name_.empty())
    BOOST_THROW_EXCEPTION(
        std::runtime_error("Current implementation only allows "
                           "primary key field appears before any "
                           "sequence field in a template."));

  if (element_instruction) {
    tables_creator nested_builder(insert_statements_, masks_, true);
    nested_builder.visit(element_instruction, nullptr);
    create_prefix_ << nested_builder.create_statements();

    if (strcmp(nested_builder.primary_key_name(), "rowid") == 0)
      create_prefix_ << "CREATE UNIQUE INDEX " << nested_builder.table_name()
                     << "_index "
                     << "ON " << nested_builder.table_name() << "("
                     << nested_builder.parameter_list() << ");\n\n";

    if (ref_instruction) {
      create_postfix_ << "CREATE TABLE " << inst->name() << "(\n";

      add_foreign_key(create_postfix_, this->table_name(),
                      this->primary_key_name(), this->primary_key_type());
      add_foreign_key(create_postfix_, nested_builder.table_name(),
                      nested_builder.primary_key_name(),
                      nested_builder.primary_key_type());
      if (is_ordered) {
        create_postfix_ << "ordering INT,\n";
      }
      create_postfix_.seekp(-2, std::ios_base::cur);
      create_postfix_ << ");\n\n";

      if (insert_statements_.count(ref_instruction->id())) {
        std::stringstream err_msg;
        err_msg << "Conflicted sequence id= " << ref_instruction->id()
                << " for " << inst->name();
        BOOST_THROW_EXCEPTION(std::runtime_error(err_msg.str()));
      }
      insert_statements &stmts = insert_statements_[ref_instruction->id()];
      stmts.insert_item_stmt =
          construct_insert_statement(inst->name(), 2 + is_ordered);
      stmts.primary_key_index = this->primary_key_index_;
    } else {
      add_foreign_key(create_current_, nested_builder.table_name(),
                      nested_builder.primary_key_name(),
                      nested_builder.primary_key_type());
      num_columns_++;

      if (is_ordered) {
        create_current_ << "  ordering INT,\n";
        num_columns_++;
      }
    }
  } else {

    tables_creator nested_builder(insert_statements_, masks_,
                                  this->table_name(), this->primary_key_name(),
                                  this->primary_key_type());
    nested_builder.visit(ref_instruction, nullptr);
    create_postfix_ << nested_builder.create_statements();
  }
}

void tables_creator::visit(const template_instruction *inst, void *) {
  table_name_ = inst->name();
  primary_key_name_ = "rowid"; // The default primary key of SQLite3

  create_current_ << "CREATE TABLE " << inst->name() << "(\n";
  std::string expression = foreign_key_expression_.str();
  create_current_ << expression;

  if (expression.size())
    num_columns_++;

  traverse_subinstructions(inst);
  // remove last inserted ",\n"
  create_current_.seekp(-2, std::ios_base::cur);
  create_current_ << ");\n\n";

  if (insert_statements_.count(inst->id())) {
    std::stringstream err_msg;
    err_msg << "Duplicated template id= " << inst->id() << " for "
            << inst->name();
    BOOST_THROW_EXCEPTION(std::runtime_error(err_msg.str()));
  }

  insert_statements &stmts = insert_statements_[inst->id()];

  stmts.insert_item_stmt =
      construct_insert_statement(inst->name(), num_columns_);
  stmts.primary_key_index = primary_key_index_;
  if (find_key_needed_ && primary_key_name_ == "rowid") {
    stmts.find_key_stmt =
        construct_find_key_statement(inst->name(), parameters_);
    stmts.primary_key_index = -1;
  }
}

void tables_creator::visit(const templateref_instruction *inst, void *) {
  create_current_ << "  " << inst->name() << " INT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const int32_vector_field_instruction *inst, void *) {
  create_current_ << "  " << inst->name() << " INT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const uint32_vector_field_instruction *inst,
                           void *) {
  create_current_ << "  " << inst->name() << " INT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const int64_vector_field_instruction *inst, void *) {
  create_current_ << "  " << inst->name() << " INT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const uint64_vector_field_instruction *inst,
                           void *) {
  create_current_ << "  " << inst->name() << " INT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

void tables_creator::visit(const enum_field_instruction *inst, void *) {
  create_current_ << "  " << inst->name() << " INT,\n";
  parameters_.push_back(inst->name());
  num_columns_++;
}

std::string tables_creator::create_statements() {
  return create_prefix_.str() + create_current_.str() + create_postfix_.str();
}

std::string tables_creator::parameter_list() {
  std::stringstream strm;
  for (std::size_t i = 0; i < parameters_.size(); ++i) {
    strm << parameters_[i] << ",";
  }
  std::string result = strm.str();
  result.resize(result.size() - 1);
  return result;
}
}

} /* mfast */
