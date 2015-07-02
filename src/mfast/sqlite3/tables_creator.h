#pragma once

#include <mfast.h>
#include <sstream>
#include <vector>

#include "field_masks.h"

namespace mfast {
namespace SQLite3 {
struct insert_statements {
  std::string insert_item_stmt;
  std::string find_key_stmt;
  int primary_key_index;
};

class tables_creator : public field_instruction_visitor {
public:
  typedef std::map<unsigned, insert_statements> insert_map_t;

  tables_creator(insert_map_t &insert_statements, const field_masks &masks,
                 const char *foreign_key_table, const char *foreign_key_name,
                 const char *primary_key_type);

  tables_creator(insert_map_t &insert_statements, const field_masks &masks,
                 bool find_key_needed = false);

  std::string create_statements();

  virtual void visit(const int32_field_instruction *, void *);
  virtual void visit(const uint32_field_instruction *, void *);
  virtual void visit(const int64_field_instruction *, void *);
  virtual void visit(const uint64_field_instruction *, void *);

  virtual void visit(const decimal_field_instruction *, void *);

  virtual void visit(const ascii_field_instruction *, void *);

  virtual void visit(const unicode_field_instruction *, void *);
  virtual void visit(const byte_vector_field_instruction *, void *);

  virtual void visit(const group_field_instruction *, void *);
  virtual void visit(const sequence_field_instruction *, void *);
  virtual void visit(const template_instruction *, void *);
  virtual void visit(const templateref_instruction *, void *);

  virtual void visit(const int32_vector_field_instruction *, void *);
  virtual void visit(const uint32_vector_field_instruction *, void *);
  virtual void visit(const int64_vector_field_instruction *, void *);
  virtual void visit(const uint64_vector_field_instruction *, void *);

  virtual void visit(const enum_field_instruction *, void *);
  const char *table_name() const;
  const char *primary_key_name() const;
  const char *primary_key_type() const;
  std::string parameter_list();
  unsigned num_columns() const { return num_columns_; }

private:
  const char *extra(const field_instruction *inst, const char *type,
                    void *pIndex);
  void add_foreign_key(std::ostream &os, const char *tablename,
                       const char *primary_key_name,
                       const char *primary_key_type);
  void traverse_subinstructions(const group_field_instruction *inst);

private:
  insert_map_t &insert_statements_;
  const field_masks masks_;
  std::stringstream create_prefix_;
  std::stringstream create_current_;
  std::stringstream create_postfix_;
  std::stringstream foreign_key_expression_;
  std::vector<const char *> parameters_;
  std::string table_name_;
  std::string primary_key_name_;
  std::string primary_key_type_;
  int primary_key_index_;
  unsigned num_columns_;
  bool find_key_needed_;
};

} /* SQLite3 */

} /* mfast */
