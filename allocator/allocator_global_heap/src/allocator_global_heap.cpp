#include <not_implemented.h>
#include "../include/allocator_global_heap.h"
#include <format>

allocator_global_heap::allocator_global_heap(
    logger *logger): _logger(logger){
        debug_with_guard("Global heap allocator constructed");
}

[[nodiscard]] void *allocator_global_heap::do_allocate_sm(
    size_t size)
    {
        size_t result_size = size + size_t_size;
        debug_with_guard("Global heap allocator started allocating " + std::to_string(result_size) + " bytes");

        void* res;

        try
        {
            res = ::operator new(result_size);
        } catch (std::bad_alloc& e)
        {
            error_with_guard("Bad alloc thrown in global_heap_allocator while trying allocate " + std::to_string(result_size) + " bytes");

            throw;
        }

        *reinterpret_cast<size_t*>(res) = size;

        debug_with_guard("Global heap allocator finished allocating " + std::to_string(result_size) + " bytes");
        return reinterpret_cast<size_t*>(res) + 1;
    }

void allocator_global_heap::do_deallocate_sm(
    void *at)
    {
        debug_with_guard("Global heap allocator started deallocating " + std::format("{}", at));
        std::string dump = get_dump(reinterpret_cast<char*>(at), *(reinterpret_cast<size_t*>(at) - 1));

        debug_with_guard(dump);

        ::delete (reinterpret_cast<size_t*>(at) - 1);
        debug_with_guard("Global heap allocator finished deallocating " + std::format("{}", at));
    }

inline logger *allocator_global_heap::get_logger() const
{
    return _logger;
}

inline std::string allocator_global_heap::get_typename() const
{
    return "allocator_global_heap";
}

allocator_global_heap::~allocator_global_heap()
{
    debug_with_guard("Global heap allocator destructor called");
}

allocator_global_heap::allocator_global_heap(const allocator_global_heap &other)
{
    _logger = other._logger;
    debug_with_guard("Global heap allocator copy constructor called");
}

allocator_global_heap &allocator_global_heap::operator=(const allocator_global_heap &other)
{
    if (this != &other)
    {
        _logger = other._logger;
        debug_with_guard("Global heap allocator copy assignment called");
    }
    return *this;
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    const allocator_global_heap* other_alloc = dynamic_cast<const allocator_global_heap*>(&other);
    return other_alloc != nullptr && _logger == other_alloc->_logger;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept : _logger(other._logger)
{
    debug_with_guard("Global heap allocator move constructor called");
}

allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept
{
    debug_with_guard("Global heap allocator move assign called");

    _logger = other._logger;

    return *this;
}
