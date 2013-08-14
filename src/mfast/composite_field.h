#ifndef COMPOSITE_FIELD_H_EK32RZA0
#define COMPOSITE_FIELD_H_EK32RZA0

#include <boost/move/core.hpp>
#include "mfast/message_ref.h"
#include "mfast/group_ref.h"
#include "mfast/sequence_ref.h"

namespace mfast
{
template <typename CRef>
class composite_field
{
  private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(composite_field)

  public:

    typedef typename CRef::instruction_cptr instruction_cptr;
    typedef CRef cref_type;
    typedef typename mref_of<cref_type>::type mref_type;

    composite_field(mfast::allocator* alloc=0,
                    instruction_cptr  instruction=0);

    // a special constructor to facilitate puting a message_type instance in an associative container
    // using emplace()
    composite_field(std::pair<mfast::allocator*,instruction_cptr> p);
    ~composite_field();

    composite_field(const cref_type & other,
                    mfast::allocator* alloc);

    composite_field(BOOST_RV_REF(composite_field) other)
      : alloc_(other.alloc_)
      , instruction_ (other.instruction_)
    {
      // g++ 4.7.1 doesn't allow this member function to defined out of class declaration
      my_storage_ = other.my_storage_;
      other.instruction_ = 0;
    }

    composite_field& operator = (BOOST_RV_REF(composite_field) other)
    {
      // g++ 4.7.1 doesn't allow this member function to defined out of class declaration
      if (this->instruction())
        this->instruction()->destruct_value(my_storage_, alloc_);

      alloc_ = other.alloc_;
      instruction_ = other.instruction_;
      my_storage_ = other.my_storage_;

      other.instruction_ = 0;
      return *this;
    }

    mref_type ref();
    mref_type mref();
    cref_type ref() const;
    cref_type cref() const;

    instruction_cptr instruction() const;
    const char* name() const;
    mfast::allocator* allocator() const;

    const value_storage* storage_for(const cref_type& other) const;

  protected:
    mfast::allocator* alloc_;
    instruction_cptr instruction_;
    value_storage my_storage_;
};

typedef composite_field<message_cref> message_type;
typedef composite_field<group_cref> group_type;
typedef composite_field<sequence_cref> sequence_type;

///////////////////////////////////////////////////////

template <typename CRef>
inline
composite_field<CRef>::composite_field(mfast::allocator*                                alloc,
                                       typename composite_field<CRef>::instruction_cptr instruction)
  : alloc_(alloc)
  , instruction_(instruction)
{
  if (instruction_)
    instruction_->construct_value(my_storage_, 0, alloc_);
}

template <typename CRef>
inline
composite_field<CRef>::composite_field(std::pair<mfast::allocator*,typename composite_field<CRef>::instruction_cptr> p)
  : alloc_(p.first)
  , instruction_(p.second)
{
  instruction_->construct_value(my_storage_, 0, alloc_);
}

template <typename CRef>
inline
composite_field<CRef>::~composite_field()
{
  if (alloc_ && this->instruction())
    this->instruction()->destruct_value(my_storage_, alloc_);
}

template <typename CRef>
inline typename composite_field<CRef>::instruction_cptr
composite_field<CRef>::instruction() const
{
  return instruction_;
}

template <typename CRef>
inline
composite_field<CRef>::composite_field(const composite_field<CRef>::cref_type& other,
                                       mfast::allocator*                       alloc)
  : alloc_(alloc)
  , instruction_(other.instruction())
{
  this->instruction()->copy_construct_value(my_storage_, 0, alloc, storage_for(other));
}

template <typename CRef>
inline typename composite_field<CRef>::mref_type
composite_field<CRef>::ref()
{
  return message_mref(alloc_, &my_storage_, instruction_);
}

template <typename CRef>
inline typename composite_field<CRef>::mref_type
composite_field<CRef>::mref()
{
  return message_mref(alloc_, &my_storage_, instruction_);
}

template <typename CRef>
inline typename composite_field<CRef>::cref_type
composite_field<CRef>::ref() const
{
  return message_cref(&my_storage_, instruction_);
}

template <typename CRef>
inline typename composite_field<CRef>::cref_type
composite_field<CRef>::cref() const
{
  return message_cref(&my_storage_, instruction_);
}

template <typename CRef>
inline const value_storage*
composite_field<CRef>::storage_for(const typename composite_field<CRef>::cref_type& other) const
{
  return detail::field_storage_helper::storage_ptr_of(other);
}

template <typename CRef>
inline const char*
composite_field<CRef>::name() const
{
  return instruction_->name();
}

template <typename CRef>
inline mfast::allocator*
composite_field<CRef>::allocator() const
{
  return this->alloc_;
}

}


#endif /* end of include guard: COMPOSITE_FIELD_H_EK32RZA0 */
