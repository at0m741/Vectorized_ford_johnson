#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

#if defined(__AVX2__)
#define PMERGEME_SIMD_AVX2 1
#else
#define PMERGEME_SIMD_AVX2 0
#endif

#if !PMERGEME_SIMD_AVX2 && (defined(__ARM_NEON) || defined(__ARM_NEON__))
#define PMERGEME_SIMD_NEON 1
#else
#define PMERGEME_SIMD_NEON 0
#endif

#if defined(PMERGEME_MODE_FAST) && defined(PMERGEME_MODE_MINCMP)
#error "Choose only one sort mode"
#endif

#if !defined(PMERGEME_MODE_FAST) && !defined(PMERGEME_MODE_MINCMP)
#define PMERGEME_MODE_FAST 1
#endif

static const uint64_t jacobsthal_table[] = {
    0, 1, 1, 3, 5, 11, 21, 43, 85, 171, 341, 683, 1365, 2731,
    5461, 10923, 21845, 43691, 87381, 174763, 349525, 699051,
    1398101, 2796203, 5592405, 11184811, 22369621, 44739243,
    89478485, 178956971, 357913941, 715827883, 1431655765,
    2863311531, 5726623061, 11453246123, 22906492245, 45812984491,
    91625968981, 183251937963, 366503875925, 733007751851, 1466015503701,
    2932031007403, 5864062014805, 11728124029611, 23456248059221, 46912496118443,
    93824992236885, 187649984473771, 375299968947541, 750599937895083, 1501199875790165,
    3002399751580331, 6004799503160661, 12009599006321323, 24019198012642645, 48038396025285291,
    96076792050570581, 192153584101141163, 384307168202282325, 768614336404564651,
    1537228672809129301, 3074457345618258603, 6148914691236517205
};

struct PairSoA {
    std::vector<int> lower;
    std::vector<int> upper;

    PairSoA() {
    }

    explicit PairSoA(size_t pair_count)
        : lower(pair_count), upper(pair_count) {
    }

    size_t size() const {
        return lower.size();
    }

    bool empty() const {
        return lower.empty();
    }
};

void check_if_sorted(const std::vector<int>& arr);
std::vector<uint64_t> generate_jacobsthal_sequence(size_t size);
PairSoA build_pair_soa(const std::vector<int>& values);
PairSoA build_pair_soa(const std::deque<int>& values);
size_t find_insertion_position(const std::vector<int>& arr, size_t size, int value);
std::vector<int> ford_johnson_sort_soa(PairSoA pairs, int straggler, bool has_straggler);
std::vector<int> ford_johnson_sort(const std::vector<int>& values, int straggler, bool has_straggler);
std::deque<int> ford_johnson_sort_deque(const std::deque<int>& values, int straggler, bool has_straggler);
void reset_comparison_count();
uint64_t get_comparison_count();
const char* get_sort_mode_name();
void check_if_sorted_deque(const std::deque<int>& arr);
