// main for allocators homework

#include "my_allocator_lib.hpp"
#include "my_vector_lib.hpp"
#include <memory>
#include <iostream>
#include <vector>
#include <map>
#include <tuple>

// Size of my allocator
constexpr std::size_t max_size = 10;

// Types shortcuts for my sized map and vector allocators
using allocator_max = my_allocator<int, max_size>;
using allocator_max_map = my_allocator<std::pair<int, int>, max_size>;

// Factorial-fill maps and vectors
template <typename T>
void fill_elements(T &cont)
{
    uint factorial = 1,
         i = 0;
    while (i < max_size)
    {
        cont[i] = factorial;
        factorial *= ++i;
    }
}

// Prints map or vector accepting element-print Lambda
template <typename T, typename Lambda>
void print_elements(T &cont, Lambda l)
{
    for (auto i : cont)
        l(i);
}

int main()
{
    try
    {
        //--------Create fill and print maps-----------
        // Prints one 'key and value'
        auto print_map_item = [](auto it)
        { std::cout << it.first << " " << it.second << "\n"; };

        std::cout << "\n"
                  << "std::map<int, int>"
                  << "\n";

        // Standard map; standard allocator
        std::map<int, int> m0;
        fill_elements(m0);
        print_elements(m0, print_map_item);

        std::cout << "\n"
                  << "map<int, int, std::less<int>, allocator_max_map>"
                  << "\n";

        // Standard map; my allocator of size 10
        std::map<int, int, std::less<int>, allocator_max_map> m1;
        fill_elements(m1);
        print_elements(m1, print_map_item);

        //-------------- Create fill and print vectors ----------
        // Prints one vector item
        auto print_vector_item = [](auto it)
        { std::cout << it << "\n"; };

        std::cout << "\n"
                  << "my_vector<int> v0(max_size)"
                  << "\n";

        // My vector 10 items; standard allocator
        my_vector<int> v0(max_size);
        fill_elements(v0);
        print_elements(v0, print_vector_item);

        std::cout << "\n"
                  << "my_vector<int, allocator_max> v1(max_size)"
                  << "\n";

        // My vector 10 items; my allocator of size 10
        my_vector<int, allocator_max> v1(max_size);
        fill_elements(v1);
        print_elements(v1, print_vector_item);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}