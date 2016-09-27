#pragma once
#include <vector>
#include <map>
#include <deque>
#include <mfast.h>
#include "tinyxml2.h"
#include "../view_iterator.h"

namespace mfast {
namespace xml_parser {
class view_info_builder : public field_instruction_visitor {
public:
  view_info_builder(mfast::arena_allocator &alloc) : alloc_(alloc) {}
  virtual void visit(const int32_field_instruction *, void *) override;
  virtual void visit(const uint32_field_instruction *, void *) override;
  virtual void visit(const int64_field_instruction *, void *) override;
  virtual void visit(const uint64_field_instruction *, void *) override;
  virtual void visit(const decimal_field_instruction *, void *) override;
  virtual void visit(const ascii_field_instruction *, void *) override;
  virtual void visit(const unicode_field_instruction *, void *) override;
  virtual void visit(const byte_vector_field_instruction *, void *) override;
  virtual void visit(const group_field_instruction *, void *) override;
  virtual void visit(const sequence_field_instruction *, void *) override;
  virtual void visit(const template_instruction *, void *) override;
  virtual void visit(const templateref_instruction *, void *) override;

  virtual void visit(const int32_vector_field_instruction *, void *) override;
  virtual void visit(const uint32_vector_field_instruction *, void *) override;
  virtual void visit(const int64_vector_field_instruction *, void *) override;
  virtual void visit(const uint64_vector_field_instruction *, void *) override;

  virtual void visit(const enum_field_instruction *inst, void *data) override;

  void print();

  aggregate_view_info build(const tinyxml2::XMLElement &element,
                            const mfast::group_field_instruction *inst);

private:
  void visit_basic(const field_instruction *inst, void *pIndex);

  void build_field_view(const tinyxml2::XMLElement &element,
                        unsigned &max_depth,
                        std::deque<field_view_info> &fields);

  typedef std::vector<int> indeces_t;

  typedef std::map<std::string, indeces_t> field_infos_t;
  mfast::arena_allocator &alloc_;
  field_infos_t infos_;
  indeces_t current_indeces_;
  std::string current_context_;
};

} /* xml_parser */
} /* mfast */
