#include "PmergeMe.hpp"

std::vector<uint64_t> generate_jacobsthal_sequence(size_t size) {
    const size_t static_size = sizeof(jacobsthal_table) / sizeof(jacobsthal_table[0]);
    const size_t limit = std::min(size, static_size);
    std::vector<uint64_t> jacobsthal(size);

    for (size_t i = 0; i < limit; ++i)
        jacobsthal[i] = jacobsthal_table[i];

    for (size_t i = static_size; i < size; ++i)
        jacobsthal[i] = jacobsthal[i - 1] + (2 * jacobsthal[i - 2]);

    return jacobsthal;
}
