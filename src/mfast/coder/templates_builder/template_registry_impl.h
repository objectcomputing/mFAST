#ifndef TEMPLATE_REGISTRY_IMPL_H_1NKEVRXF
#define TEMPLATE_REGISTRY_IMPL_H_1NKEVRXF

#include <map>
#include <string>
#include "../dynamic_templates_description.h"

namespace mfast {
namespace coder {

struct template_registry_impl
{
  typedef std::map<std::string, const template_instruction*> map_type;
  map_type templates_;
  arena_allocator alloc_;


  std::string get_key(const char* ns, const char* name) const
  {
    return std::string(ns) + "||" + name;
  }

  const template_instruction*
  find(const char* ns, const char* name) const
  {
    map_type::const_iterator itr = templates_.find(get_key(ns, name));
    if (itr != templates_.end()) {
      return itr->second;
    }
    return 0;
  }

  void add(const char* ns, const template_instruction* inst)
  {
    templates_[get_key(ns, inst->name())] = inst;
  }

};

}
}
#endif /* end of include guard: TEMPLATE_REGISTRY_IMPL_H_1NKEVRXF */
