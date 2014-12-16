#ifndef TEMPLATE_REPO_H_5C1F7279
#define TEMPLATE_REPO_H_5C1F7279

#include "dictionary_builder.h"
#include <boost/container/map.hpp>

namespace mfast {


template <typename T>
struct template_repo_mapped_type;

class template_repo_base
{
public:

  template_repo_base(mfast::allocator* dictionary_alloc)
    : dictionary_alloc_(dictionary_alloc)
  {
  }

  virtual ~template_repo_base()
  {
    for (std::size_t i = 0; i < vector_enties_.size(); ++i) {
      if (vector_enties_[i]->of_array.capacity_in_bytes_)
        dictionary_alloc_->deallocate(vector_enties_[i]->of_array.content_,
                                      vector_enties_[i]->of_array.capacity_in_bytes_);
    }
  }

  void reset_dictionary()
  {
    for (std::size_t i = 0; i < reset_entries_.size(); ++i) {
      reset_entries_[i]->defined(false);
    }
  }

  virtual template_instruction* get_template(uint32_t id) = 0;

private:

  void add_reset_entry(value_storage* entry)
  {
    reset_entries_.push_back(entry);
  }

  void add_vector_entry(value_storage* entry)
  {
    if (dictionary_alloc_)
      vector_enties_.push_back(entry);
  }

protected:
  friend class dictionary_builder;

  typedef std::vector<value_storage*> value_entries_t;
  value_entries_t reset_entries_;
  value_entries_t vector_enties_;   // for string and byteVector
  arena_allocator instruction_alloc_;
  mfast::allocator* dictionary_alloc_;
};



template <typename EntryValueConverter>
class template_repo
  : public template_repo_base
{
  typedef typename EntryValueConverter::repo_mapped_type repo_mapped_type;
  typedef boost::container::map<uint32_t, repo_mapped_type> templates_map_t;

  struct repo_entry_inserter
  {
    template_repo<EntryValueConverter>* repo_;
    repo_entry_inserter(template_repo<EntryValueConverter>* repo)
      : repo_(repo)
    {
    }

    void operator() (template_instruction* inst) const
    {
      repo_->add_template(inst, (void*) 0);
    }

    template <typename Message>
    void operator() (template_instruction* inst, Message* msg) const
    {
      repo_->add_template(inst, msg);
    }

  };

public:
  template_repo(mfast::allocator* dictionary_alloc=0)
    : template_repo_base(dictionary_alloc)
  {
  }

  template_repo(EntryValueConverter converter,
                mfast::allocator*   dictionary_alloc=0)
    : template_repo_base(dictionary_alloc)
    , converter_(converter)
  {
  }

  void build(const templates_description** descriptions,
             std::size_t                   description_count)
  {
    dictionary_builder builder(*this);
    repo_entry_inserter inserter(this);

    for (std::size_t i = 0; i < description_count; ++i)
      builder.build(descriptions[i], inserter);
  }

  template <typename Iterator>
  void build(Iterator first,
             Iterator last)
  {
    dictionary_builder builder(*this);
    repo_entry_inserter inserter(this);

    for (Iterator i = first; i != last; ++i)
      builder.build(static_cast<const templates_description*>(&(*i)), inserter);
  }


  template <typename DescriptionTuple>
  void build(const DescriptionTuple& tp)
  {
    dictionary_builder builder(*this);
    builder.build( tp, repo_entry_inserter(this) );
  }

  repo_mapped_type* find(uint32_t id)
  {
    typename templates_map_t::iterator it = templates_map_.find(id);
    if (it != templates_map_.end())
      return &it->second;
    return 0;
  }

  template_instruction* get_template(uint32_t id)
  {
    repo_mapped_type* entry = this->find(id);
    if (entry) {
      return converter_.to_instruction( *entry );
    }
    return 0;
  }

  repo_mapped_type* unique_entry()
  {
    if (templates_map_.size() == 1) {
      return &templates_map_.begin()->second;
    }
    return 0;
  }

  template <typename Message>
  void add_template(template_instruction* inst, Message* msg)
  {
    uint32_t id = inst->id();
    // assert(dynamic_cast<const typename Message::instruction_type*>(templates_map_[Message::the_id]) );
    this->templates_map_.emplace(id, converter_.to_repo_entry(inst, msg));
  }



protected:
  templates_map_t templates_map_;
  EntryValueConverter converter_;
};

struct trivial_template_repo_entry_converter
{
  typedef template_instruction* repo_mapped_type;
  template_instruction*
  to_instruction(template_instruction* inst) const
  {
    return inst;
  }

  template_instruction*
  to_repo_entry(template_instruction* inst, void*) const
  {
    return inst;
  }
};

typedef template_repo<trivial_template_repo_entry_converter> simple_template_repo_t;


template <typename T, typename MappedType, typename MappedTypeConstructionTuple>
struct template_repo_entry_converter
{
  typedef MappedType repo_mapped_type;
  T* impl_;

  template_repo_entry_converter(T* impl)
    : impl_(impl)
  {
  }

  template_instruction*
  to_instruction(const repo_mapped_type& entry) const
  {
    return impl_->to_instruction(entry);
  }

  template <typename Message>
  MappedTypeConstructionTuple
  to_repo_entry(template_instruction* inst, Message* msg) const
  {
    return impl_->to_repo_entry(inst, msg);
  }

};

}

#endif /* end of include guard: TEMPLATE_REPO_H_5C1F7279 */
