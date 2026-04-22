#include "PmergeMe.hpp"

std::deque<int> ford_johnson_sort_deque(const std::deque<int>& values, int straggler, bool has_straggler) {
    reset_comparison_count();
    const std::vector<int> sorted = ford_johnson_sort_soa(build_pair_soa(values), straggler, has_straggler);
    return std::deque<int>(sorted.begin(), sorted.end());
}
