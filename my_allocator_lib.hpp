// This is an STL wrapper for raw memory allocator
#pragma once
#include "raw_pool_lib.hpp"
//
template <class T, size_t pool_size>
struct my_allocator
{
    // Raw pool size is calculated as nof items * (size of an item + occupied memory header)
    raw_pool<pool_size * (sizeof(T) + raw_pool<pool_size>::overhead)> rpool;

    using value_type = T;

    // Default cnstr
    my_allocator() noexcept {}
    // Copy cnstr
    my_allocator(const my_allocator<T, pool_size> &other) noexcept
    {
        raw_pool(other.rpool);
    }
    // Cnstr for container nodes, if any
    template <class U>
    my_allocator(const my_allocator<U, pool_size> &a)
    {
        raw_pool(a.rpool);
    }

    ~my_allocator() noexcept {}

    T *allocate(std::size_t n)
    {
        return static_cast<T *>(rpool.allocate(n * sizeof(T)));
    }
    void deallocate(T *p, [[maybe_unused]] std::size_t n)
    {
        rpool.deallocate(p);
    }

    // Allocator traits must have allready implement rebind
    // but I couldn't compile without the following template
    template <class U>
    struct rebind
    {
        typedef my_allocator<U, pool_size> other;
    };

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;
};

template <class T, class U, std::size_t pool_size>
constexpr bool operator==(const my_allocator<T, pool_size> &a1, const my_allocator<U, pool_size> &a2) noexcept
{
    return std::tie(a1.pool.data, a1.pool.count, a1.pool.first_free_block) == std::tie(a2.pool.data, a2.pool.count, a2.pool.first_free_block);
}

template <class T, class U, std::size_t pool_size>
constexpr bool operator!=(const my_allocator<T, pool_size> &a1, const my_allocator<U, pool_size> &a2) noexcept
{
    return std::tie(a1.pool.data, a1.pool.count, a1.pool.first_free_block) != std::tie(a2.pool.data, a2.pool.count, a2.pool.first_free_block);
}
