#pragma once

#include <initializer_list>
#include <cstring>
#include <memory>

template <class T>
class Iterator
{
public:
    Iterator() : ptr(nullptr) {}

    Iterator(T *p) : ptr(p) {}

    Iterator(const Iterator &it) : ptr(it.ptr) {}

    Iterator &operator=(const Iterator &it)
    {
        ptr = it.ptr;
        return *this;
    }

    bool operator==(const Iterator &it) const
    {
        return (ptr == it.ptr);
    }

    bool operator!=(const Iterator &it) const
    {
        return !(it == *this);
    }

    Iterator &operator++()
    {
        ++ptr;
        return *this;
    }

    Iterator &operator--()
    {
        --ptr;
        return *this;
    }

    T &operator*() const
    {
        return *ptr;
    }

private:
    T *ptr;
};

template <class T, class Allocator = std::allocator<T>>
class my_vector
{
private:
    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;

    std::size_t first_free_idx;
    std::size_t _capacity;
    T *data = nullptr;
    Allocator alloc;

    void reserve(const std::size_t capacity);

public:
    my_vector() : first_free_idx(0), _capacity(0){};
    my_vector(const my_vector<T, Allocator> &another_vector);
    my_vector(const my_vector<T, Allocator> &&another_vector);
    my_vector<T, Allocator> &operator=(const my_vector<T, Allocator> &);
    my_vector<T, Allocator>(std::size_t capacity, T initial = T{});

    ~my_vector()
    {
        std::allocator_traits<Allocator>::deallocate(alloc, data, first_free_idx);
    }
    std::size_t size() const { return first_free_idx; }
    std::size_t capacity() const { return _capacity; }
    bool empty() const { return first_free_idx == 0; }

    T &operator[](const std::size_t index);
    void emplace_back(const T &element);
    T pop();

    iterator begin() noexcept { return iterator(data); }

    iterator end() noexcept { return iterator(data + first_free_idx); }
};

template <class T, class Allocator>
T &my_vector<T, Allocator>::operator[](const std::size_t index)
{
    if (index >= first_free_idx)
        throw std::invalid_argument("Index must be less than vector's size");

    return data[index];
}

template <class T, class Allocator>
my_vector<T, Allocator>::my_vector(const my_vector<T, Allocator> &another)
{

    std::allocator_traits<Allocator>::deallocate(alloc, data, first_free_idx);

    first_free_idx = another.size();
    _capacity = another.capacity();
    data = std::allocator_traits<Allocator>::allocate(alloc, _capacity);
    std::memcpy(data, another.data, first_free_idx * sizeof(T));
}

template <class T, class Allocator>
my_vector<T, Allocator> &my_vector<T, Allocator>::operator=(const my_vector<T, Allocator> &another)
{
    auto tmp = new my_vector<T, Allocator>(another);

    std::allocator_traits<Allocator>::deallocate(alloc, data, first_free_idx);
    first_free_idx = tmp->size();
    _capacity = tmp->capacity();

    data = std::allocator_traits<Allocator>::allocate(alloc, _capacity);
    std::memcpy(data, tmp->data, first_free_idx * sizeof(T));
    return *this;
}

template <class T, class Allocator>
my_vector<T, Allocator>::my_vector(std::size_t capacity, T initial) : first_free_idx(capacity), _capacity(capacity)

{
    data = std::allocator_traits<Allocator>::allocate(alloc, capacity);

    for (std::size_t i = 0; i < capacity; ++i)
        data[i] = initial;
}

template <class T, class Allocator>
void my_vector<T, Allocator>::emplace_back(const T &element)
{
    // If no capacity, increase capacity
    if (first_free_idx == _capacity)
    {
        if (_capacity == 0)
            reserve(8);
        else
            reserve(_capacity * 2);
    }

    // Append an element to the array
    new (data + first_free_idx) T(element);
    first_free_idx++;
}

template <class T, class Allocator>
T my_vector<T, Allocator>::pop()
{
    if (first_free_idx > 0) // Nothing to pop otherwise
    {
        T to_return = data[first_free_idx - 1]; // store return value before deleting
        first_free_idx--;

        return to_return;
    }
    else
        throw std::out_of_range("Nothing to pop");
}

template <class T, class Allocator>
void my_vector<T, Allocator>::reserve(const std::size_t capacity)
{
    if (capacity > first_free_idx)
    {
        // Reserves memory of size capacity for data
        T *temp = std::allocator_traits<Allocator>::allocate(alloc, capacity);

        // Move previous elements to this memory
        for (std::size_t i = 0; i < _capacity; ++i)
            temp[i] = data[i];

        std::allocator_traits<Allocator>::deallocate(alloc, data, first_free_idx);
        _capacity = capacity;
        data = temp;
    }
}
