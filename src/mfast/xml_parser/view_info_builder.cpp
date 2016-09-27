#include <cstring>
#include <string.h>
#include "view_info_builder.h"
#include "../exceptions.h"
#include <algorithm>
namespace mfast {
namespace xml_parser {

void view_info_builder::visit_basic(const field_instruction *inst,
                                    void *pIndex) {
  std::size_t current_context_size = current_context_.size();
  if (current_context_size)
    current_context_ += ".";
  current_context_ += inst->name();

  current_indeces_.push_back(
      static_cast<int>(*static_cast<std::size_t *>(pIndex)));
  infos_[current_context_] = current_indeces_;

  current_indeces_.pop_back();
  current_context_.resize(current_context_size);
}

void view_info_builder::visit(const group_field_instruction *inst,
                              void *pIndex) {
  std::size_t current_context_size = current_context_.size();

  if (pIndex) {
    current_indeces_.push_back(
        static_cast<int>(*static_cast<std::size_t *>(pIndex)));
    if (current_context_size)
      current_context_ += ".";
    current_context_ += inst->name();

    infos_[current_context_] = current_indeces_;
  }
  std::size_t i = 0;
  for (auto subinst : inst->subinstructions()) {
    subinst->accept(*this, &i);
    ++i;
  }

  if (pIndex) {
    current_indeces_.pop_back();
    current_context_.resize(current_context_size);
  }
}

void view_info_builder::visit(const sequence_field_instruction *inst,
                              void *pIndex) {
  std::size_t current_context_size = current_context_.size();
  if (current_context_size)
    current_context_ += ".";

  current_context_ += inst->name();
  current_indeces_.push_back(
      pIndex ? static_cast<int>(*static_cast<std::size_t *>(pIndex)) : 0);
  infos_[current_context_] = current_indeces_;

  // std::cout << "inserting " << current_context_ << "\n";

  current_context_ += "[]";
  current_indeces_.push_back(-2); // place holder for sequence element index
  infos_[current_context_] = current_indeces_;

  // std::cout << "inserting " << current_context_ << "\n";

  std::size_t i = 0;
  for (auto subinst : inst->subinstructions()) {
    subinst->accept(*this, &i);
    ++i;
  }

  current_indeces_.pop_back();
  current_indeces_.pop_back();
  current_context_.resize(current_context_size);
}

void view_info_builder::visit(const int32_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const uint32_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const int64_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const uint64_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const decimal_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const ascii_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const unicode_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const byte_vector_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const template_instruction *inst, void *pIndex) {
  visit(static_cast<const group_field_instruction *>(inst), pIndex);
}

void view_info_builder::visit(const templateref_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const int32_vector_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const uint32_vector_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const int64_vector_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const uint64_vector_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

void view_info_builder::visit(const enum_field_instruction *inst,
                              void *pIndex) {
  visit_basic(inst, pIndex);
}

struct tag_reference_name;
typedef boost::error_info<tag_reference_name, std::string> reference_name_info;

inline const char *get_optional_attr(const tinyxml2::XMLElement &element,
                                     const char *attr_name,
                                     const char *default_value) {
  const tinyxml2::XMLAttribute *attr = element.FindAttribute(attr_name);
  if (attr == nullptr) {
    return default_value;
  }
  return attr->Value();
}

void view_info_builder::build_field_view(const tinyxml2::XMLElement &element,
                                         unsigned &max_depth,
                                         std::deque<field_view_info> &fields) {
  std::string ref_name = get_optional_attr(element, "name", "");
  if (ref_name.size()) {
    field_view_info result = {field_view_info::CONTINUE_BIT, nullptr};

    std::vector<int> sequence_indeces;

    std::string ref_name_no_seq_index;
    // remove anything in [] and save the numbers in [] into a queue

    std::size_t pos = 0, new_pos;
    do {
      // find the first occurence of '['
      new_pos = ref_name.find_first_of('[', pos);
      if (new_pos != std::string::npos) {
        ++new_pos;
        ref_name_no_seq_index += ref_name.substr(pos, (new_pos - pos));
        pos = new_pos;
        new_pos = ref_name.find_first_of(']', pos);
        if (new_pos != std::string::npos) {
          try {
            sequence_indeces.push_back(boost::lexical_cast<unsigned>(
                ref_name.substr(pos, new_pos - pos)));
            pos = new_pos;
          } catch (boost::bad_lexical_cast &) {
            BOOST_THROW_EXCEPTION(
                fast_static_error("Invalid reference specification, the token "
                                  "inside [] must be a non-negative integer")
                << reference_name_info(ref_name));
          }
        }
      } else {
        ref_name_no_seq_index += ref_name.substr(pos);
      }
    } while (new_pos != std::string::npos);
    // find the field index
    // std::cout << "finding " << ref_name_no_seq_index << "\n";
    auto it = infos_.find(ref_name_no_seq_index);
    if (it == infos_.end()) {
      BOOST_THROW_EXCEPTION(
          fast_static_error(
              "Invalid reference specification, no such reference name exists")
          << reference_name_info(ref_name));
    }
    const indeces_t &field_indeces = it->second;
    // replace every instance -2 with the indeces in []
    typedef field_view_info::nest_index_t nest_index_t;
    nest_index_t *nest_indices = static_cast<nest_index_t *>(
        alloc_.allocate(sizeof(nest_index_t) * field_indeces.size() + 1));

    unsigned i = 0, j = 0;
    for (i = 0; i < field_indeces.size(); ++i) {
      if (field_indeces[i] != -2)
        nest_indices[i] = field_indeces[i];
      else {
        // -2 is used for specifying undefined sequence index
        if (j < sequence_indeces.size())
          nest_indices[i] = sequence_indeces[j++];
        else {
          BOOST_THROW_EXCEPTION(
              fast_static_error("Invalid reference specification, no such "
                                "reference name exists")
              << reference_name_info(ref_name));
        }
      }
    }
    // the nest_indeces must terminiated with a -1
    nest_indices[i] = -1;
    result.nest_indices = nest_indices;
    max_depth = std::max<unsigned>(max_depth,
                                   static_cast<unsigned>(field_indeces.size()));
    result.prop += 1;
    if (fields.size()) {
      result.prop += std::mismatch(nest_indices, nest_indices + i,
                                   fields.back().nest_indices)
                         .first -
                     nest_indices;
    }
    fields.push_back(result);
  }
}

aggregate_view_info
view_info_builder::build(const tinyxml2::XMLElement &element,
                         const mfast::group_field_instruction *inst) {
  const char *name = get_optional_attr(element, "name", nullptr);

  if (name == nullptr)
    BOOST_THROW_EXCEPTION(
        fast_static_error("A view must has a name attribute"));

  this->visit(inst, nullptr);
  aggregate_view_info result;
  result.max_depth_ = 0;
  std::size_t sz = std::strlen(name) + 1;
  result.name_ = reinterpret_cast<const char *>(
      std::memcpy(new (alloc_) char[sz], name, sz));

  // result.name_ = std::strcpy(new (alloc_) char[std::strlen(name) + 1],
  //                            name);

  std::deque<field_view_info> fields;

  const tinyxml2::XMLElement *child = element.FirstChildElement();
  while (child != nullptr) {
    if (std::strcmp(child->Name(), "field") == 0) {
      const tinyxml2::XMLElement *grandchild = child->FirstChildElement();
      while (grandchild != nullptr) {
        build_field_view(*grandchild, result.max_depth_, fields);
        grandchild = grandchild->NextSiblingElement();
      }

      fields.back().prop &= ~field_view_info::CONTINUE_BIT;
    }
    child = child->NextSiblingElement();
  }
  field_view_info terminator = {0, nullptr};
  fields.push_back(terminator);

  auto data = new (alloc_) field_view_info[fields.size()];
  std::copy(fields.begin(), fields.end(), data);
  result.data_ = mfast::array_view<const field_view_info>(data, fields.size());
  result.instruction_ = inst;
  return result;
}

void view_info_builder::print() {
  for (auto v : infos_) {
    std::cout << v.first << "=>";
    for (auto &elem : v.second)
      std::cout << elem << ", ";
    std::cout << "\n";
  }
}

} /* xml_parser */
} /* mfast */
