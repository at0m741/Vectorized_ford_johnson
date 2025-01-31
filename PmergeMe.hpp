#pragma once

#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <iostream>
#include <stdint.h>
#include <ctime>
#include <deque>
#include <immintrin.h>
#include <stdlib.h>
#include <dlfcn.h>

#define align	__attribute__((aligned(32)))
#define SYM		0x0000000065657266

struct alignas(32) PairArray {
    int* keys;    // Correspond à `pairs.first`
    int* values;  // Correspond à `pairs.second`
    size_t size;

    PairArray(size_t n) : size(n) {
        keys = static_cast<int*>(aligned_alloc(32, n * sizeof(int)));
        values = static_cast<int*>(aligned_alloc(32, n * sizeof(int)));
    }

    ~PairArray() {
        free(keys);
        free(values);
    }
};


align static const uint64_t jacobsthal_table[] = {
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


struct ComparePairs 
{
    bool operator()(const std::pair<int, int>& a, const std::pair<int, int>& b) const 
	{
        return a.second < b.second;
    }
} align;

__attribute__((always_inline, hot))
inline __m256i load_256(const void* ptr) {
    if (!((uint64_t)ptr & 31)) {
        return _mm256_load_si256((__m256i*)ptr);
    } else {
        return _mm256_loadu_si256((__m256i*)ptr);
    }
}

__attribute__((always_inline, hot))
inline void store_256(void* ptr, __m256i data) {
    if (!((uint64_t)ptr & 31)) {
        _mm256_store_si256((__m256i*)ptr, data);
    } else {
        _mm256_storeu_si256((__m256i*)ptr, data);
    }
}

template <class Compare>
__attribute__((always_inline, hot))
inline void merge4_unrolled(
    std::vector<std::pair<int,int> >& pairs,
    const std::vector<std::pair<int,int> >& leftArray,
    const std::vector<std::pair<int,int> >& rightArray,
    int &i,
    int &j,
    int &k,
    Compare compare) {
        bool c = compare(leftArray[i], rightArray[j]);
        pairs[k]    = c ? leftArray[i] : rightArray[j];
        i          += c ? 1 : 0;
        j          += c ? 0 : 1;
        ++k;
}

void* check_alignment(void* ptr, std::size_t alignment);
bool is_aligned_32(const void* ptr);
void calculate_jacobsthal_avx(std::vector<uint64_t>& jacobsthal, size_t start, size_t size); 
void check_if_sorted(const std::vector<int>& arr);
std::vector<uint64_t> generate_jacobsthal_AVX(size_t size);
void compare_pairs_avx(std::vector<std::pair<int, int> >& pairs);
void insertion(std::vector<int>& arr, int value);
std::vector<int> ford_johnson_sort(std::vector<std::pair<int, int> >& pairs, int straggler, bool has_straggler) ;
std::deque<int> ford_johnson_sort_deque(std::deque<std::pair<int, int> >& pairs, int straggler, bool has_straggler); 

void insertion_deque(std::deque<int>& arr, int value);
void compare_pairs_avx_deque(std::deque<std::pair<int, int> >& pairs);
void calculate_jacobsthal_deque(std::deque<uint64_t>& jacobsthal, size_t start, size_t size); 
std::deque<uint64_t> generate_jacobsthal_deque(size_t size); 
void check_if_sorted_deque(const std::deque<int>& arr);
