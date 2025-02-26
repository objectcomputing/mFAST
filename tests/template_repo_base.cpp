#include "catch.hpp"

#define TESTING_TEMPLATE_REPO_BASE
#include <mfast/coder/common/template_repo.h>
#include <mfast/value_storage.h>
#include <mfast/malloc_allocator.h>
#include <mfast/instructions/template_instruction.h>
#include <mfast/coder/common/dictionary_builder.h>

class template_repo_base_impl : public mfast::template_repo_base
{
public:
    template_repo_base_impl(mfast::allocator *dictionary_alloc)
        : mfast::template_repo_base(dictionary_alloc) {}

    virtual mfast::template_instruction *get_template(uint32_t)
    {
        return nullptr;
    }
};

TEST_CASE("Test template repo base with repeated pointers","[test_template_repo_base]")
{
    using namespace mfast;

    value_storage* storage_1_ =  new value_storage();
    {
        const char * test_data = "Test_Data";
        storage_1_->of_array.len_ = static_cast<uint32_t>(std::strlen(test_data) + 1);
        storage_1_->of_array.content_ = malloc_allocator::instance()->allocate(storage_1_->of_array.len_);
        storage_1_->of_array.capacity_in_bytes_ = 9;
    }

    {
        template_repo_base_impl templ_repo(malloc_allocator::instance());
        templ_repo.add_vector_entry(storage_1_);
        templ_repo.add_vector_entry(storage_1_);
    }

    delete storage_1_;
}
