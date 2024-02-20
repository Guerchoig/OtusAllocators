// This is an unoptimized non-concurrent non-STL memory allocator,
// with a unidirectional list of free blocks,
// written in educational purposes
#pragma once

#include "derive_type_lib.hpp"
#include <cstdint>
#include <limits>
#include <tuple>
#include <stdexcept>
#include <new>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include <limits>

#ifndef NDEBUG
constexpr uint print_buf_len = 30;
#endif

// A size of a small buffer, located after the pool memory,
// (one can never be safe with raw pointers)
constexpr uint safety_buf = 16; // bytes

// The main pool class
template <std::size_t p_pool_size>
struct raw_pool
{
	static constexpr std::size_t pool_size = p_pool_size;

	// The type metrics_t, defining distances in the pool
	// is derived from the maximum pool size
	using metrics_t = typename st<nof_bytes(pool_size)>::type;

	// Free and occupied block's headers forward defs
	struct memory_block;
	struct free_block;
	struct occ_block;

	// Pool state control variables
	uint8_t *data = nullptr;
	metrics_t first_free_block = 0;
	metrics_t count = 0;

	// Some openers
	template <typename T>
	T *p_block(const metrics_t &offset)
	{
		return reinterpret_cast<T *>(data + offset);
	}

	// Parent block header
	struct memory_block
	{
		metrics_t block_size;
	};

	// A publically accessible maximum overhead for single allocation
	// needed for container could safely calculate the pool size
	static constexpr auto overhead = sizeof(memory_block);

	// "Size" field in any block header
	metrics_t &bl_size(const metrics_t &offset)
	{
		return p_block<memory_block>(offset)->block_size;
	}

	// Free block header
	struct free_block : memory_block
	{
		metrics_t block_next; // uses -1 to mark the last
	};

	// "Next" field in free block header
	metrics_t &bl_next(const metrics_t &offset)
	{
		return p_block<free_block>(offset)->block_next;
	}

	// Occupied block header
	struct occ_block : memory_block
	{
	};

	// Block offset by pointer
	metrics_t bl_offset(void *ptr)
	{
		return static_cast<uint8_t *>(ptr) - data - sizeof(occ_block);
	}

	// Try to find free block of appropriate size at given offset
	// by possibly merging the current block with following blocks
	bool local_match_found(const metrics_t offset, const metrics_t msize)
	{
		auto curr = offset;
		for (; bl_size(curr) <= pool_size; curr = bl_next(curr))
		{
			// If current block matches - OK
			if (bl_size(offset) >= msize)
				return true;

			// If the next block is also free, then merge with it
			if (bl_next(curr) == curr + sizeof(memory_block) + bl_size(curr))
			{
				bl_size(curr) += (bl_size(bl_next(curr)) + sizeof(memory_block));
				bl_next(curr) = bl_next(bl_next(curr));
				count += sizeof(memory_block);
				std::cout << "Merge:\n";
#ifndef NDEBUG
				print_pool(50);
#endif
			}
		}
		return false;
	}
	// Default constructor
	raw_pool()
	{
		data = static_cast<decltype(data)>(malloc(pool_size + sizeof(metrics_t) + safety_buf));
		if (!data)
			throw std::bad_alloc();
		first_free_block = 0;
		bl_size(first_free_block) = static_cast<metrics_t>(pool_size);
		bl_next(first_free_block) = static_cast<metrics_t>(pool_size) + sizeof(metrics_t);
		count = pool_size;
	}
	// Copy constructor
	raw_pool(const raw_pool<pool_size> &pool) : first_free_block(pool.first_free_block),
												count(pool.count)
	{
		data = static_cast<decltype(data)>(malloc(pool_size + sizeof(metrics_t) + safety_buf));
		if (!data)
			throw std::bad_alloc();
		memcpy(data, pool.data, pool_size + sizeof(metrics_t) + safety_buf);
	}
	// Rvalue constructor
	raw_pool(raw_pool<pool_size> &&rhs) : data(std::move(rhs.data)),
										  first_free_block(rhs.first_free_block),
										  count(rhs.count) {}

	~raw_pool()
	{
		free(data);
	}

	raw_pool &operator=(const raw_pool &) = delete;

	// Estimates the whole pool free space, avalable for one time allocation
	// ignoring possible fragmentation
	bool has_space(size_t alloc_size)
	{
		return count >= (alloc_size + sizeof(memory_block));
	}

	/// @brief Allocate memory of given size or throws exception if failed
	/// @param alloc_size size of memory to allocate
	/// @return void ptr to allocated memory
	[[nodiscard]] void *allocate(size_t alloc_size)
	{
		// If there is no free space
		if (!has_space(alloc_size))
			throw std::bad_alloc();

		// Search for a continuous piece of memory
		auto curr = first_free_block;
		for (auto prv = curr; curr < pool_size; curr = bl_next(prv))
		{
			// Search for free memory piece at curr offset
			if (local_match_found(curr, alloc_size))
			{
				// If the found block is too large and is worth to be divided
				if (bl_size(curr) > alloc_size + sizeof(free_block))
				{
					metrics_t new_free_block = curr + sizeof(occ_block) + alloc_size;
					if (curr == first_free_block)
						first_free_block = new_free_block;
					else
						bl_next(prv) = new_free_block;

					bl_next(new_free_block) = bl_next(curr);
					bl_size(new_free_block) = bl_size(curr) - alloc_size - sizeof(occ_block);
					bl_size(curr) = alloc_size;
					count -= (alloc_size + sizeof(memory_block));
				}
				// If the found block fits exactly
				else
				{
					bl_next(prv) = bl_next(curr);
					if (curr == first_free_block)
						first_free_block = bl_next(curr);
					else
						bl_next(prv) = bl_next(curr);
					count -= alloc_size;
				}
				// Success
				return p_block<void *>(curr + sizeof(occ_block));
			}
			prv = curr;
		}
		// A continuous piece of memory is not found
		throw std::bad_alloc();
	}

	// Is used when deallocating to keep the chain of free blocks connected
	metrics_t find_previous_free_block(metrics_t offset)
	{
		for (auto prv = first_free_block; prv < offset; prv = bl_next(prv))
			if (bl_next(prv) > offset)
				return prv;
		throw std::bad_alloc();
	}

	// Gives back the memory, allocated at ptr
	void deallocate(void *ptr)
	{
		if (!in_bounds(ptr))
			throw std::bad_alloc();

		metrics_t offset = bl_offset(ptr);
		if (first_free_block < offset)
		{
			// Merges given back memory with the preceeding free block
			auto prev = find_previous_free_block(offset);
			bl_next(offset) = bl_next(prev);
			bl_next(prev) = offset;
			auto add_size = bl_size(offset) + sizeof(occ_block);
			bl_size(prev) += add_size;
			count += add_size;
		}
		else
		{
			// Just transform an occupied block into a free one
			// without merging
			bl_next(offset) = first_free_block;
			first_free_block = offset;
			count += bl_size(offset);
		}
	}

	// If it's this pool pointer or a wrong ptr?
	bool in_bounds(void *ptr) const noexcept
	{
		auto bptr = reinterpret_cast<uint8_t *>(ptr);
		return (bptr >= data) && (bptr < data + pool_size);
	}

#ifndef NDEBUG
	// Prints out the pool from zero offset up to max_len
	void print_pool(metrics_t max_len)
	{
		std::cout << "Count:" << count << "\n";
		std::cout << "First free block:" << first_free_block << "\n";

		auto next_free_block = first_free_block;
		metrics_t pos = 0;
		while (pos < max_len)
		{
			// Print one block -------------------------------------
			auto _size = bl_size(pos);

			std::cout << std::dec << pos << ": " << _size;
			if (pos == next_free_block)
			{
				next_free_block = bl_next(pos);
				std::cout << " " << next_free_block << " free";
			}

			std::cout << std::endl;

			pos += sizeof(memory_block);
			auto start = pos;
			std::cout << start << ": ";
			while (pos < start + _size && pos < max_len)
				std::cout
					<< std::dec << +data[pos++] << " ";

			std::cout << std::endl;
		};
	}
#endif
};