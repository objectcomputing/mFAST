#ifndef FIELD_BUILDER_BASE_H_CGMKXJYG
#define FIELD_BUILDER_BASE_H_CGMKXJYG

#include <deque>
#include <map>
#include "mfast/field_instruction.h"
#include "xml_util.h"
#include "template_registry_impl.h"
namespace mfast
{
namespace coder
{
using namespace tinyxml2;

typedef std::map<std::string, const_instruction_ptr_t> type_map_t;

class field_builder_base
{
public:

  field_builder_base(template_registry_impl* registry,
                     const type_map_t*       types)
    : registered_templates_(registry)
    , registered_types_(types)
    , resolved_ns_(0)
    , is_optional_group_(false)
  {
  }

  bool is_optional_group() const
  {
    return is_optional_group_;
  }

  const char* resolved_ns() const
  {
    return resolved_ns_;
  }

  const char* get_typeRef_name(const XMLElement & element)
  {
    const XMLElement* typeRefElem = element.FirstChildElement("typeRef");
    if (typeRefElem) {
      return string_dup(get_optional_attr(*typeRefElem, "name", ""), alloc());
    }
    return "";
  }

  const char* get_typeRef_ns(const XMLElement & element)
  {
    const XMLElement* typeRefElem = element.FirstChildElement("typeRef");
    if (typeRefElem) {
      return string_dup(get_optional_attr(*typeRefElem, "ns", ""), alloc());
    }
    return "";
  }

  arena_allocator& alloc() const
  {
    return registered_templates_->alloc_;
  }

  const_instruction_ptr_t find_type(const char* type_name) const
  {
    type_map_t::const_iterator itr = registered_types_->find(std::string(type_name));
    if (itr != registered_types_->end()) {
      return itr->second;
    }
    return 0;
  }

  const template_instruction*
  find_template(const char* ns, const char* name) const
  {
    return registered_templates_->find(ns, name);
  }

  virtual const char* name() const = 0;

  virtual std::size_t num_instructions() const = 0;
  virtual void add_instruction(const field_instruction*)=0;
  virtual void add_instruction(const group_field_instruction*)=0;

  virtual void add_template(const char* ns, template_instruction* inst)=0;

  template_registry_impl* registered_templates() const
  {
    return registered_templates_;
  }

  const type_map_t* registered_types() const
  {
    return registered_types_;
  }

protected:
  template_registry_impl* registered_templates_;
  const type_map_t* registered_types_;
  const char* resolved_ns_;
  bool is_optional_group_;
};


}   /* coder */

} /* mfast */


#endif /* end of include guard: FIELD_BUILDER_BASE_H_CGMKXJYG */
