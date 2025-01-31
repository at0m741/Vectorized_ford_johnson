#include "PmergeMe.hpp"
#include <xmmintrin.h>

int comparison_count = 0;
thread_local std::vector<std::pair<int, int>> tls_left_buffer, tls_right_buffer;


__attribute__((always_inline, hot))
static inline void merge_pairs(std::vector<std::pair<int, int> >& pairs, int left, int middle, int right) {
    int n1 = middle - left + 1;
    int n2 = right - middle;
    int i = 0, j = 0, k = left;
    std::vector<std::pair<int, int> > leftArray(n1);
    std::vector<std::pair<int, int> > rightArray(n2);
	leftArray.resize(n1);
	rightArray.resize(n2);

	if (n1 > 0) 
		__builtin_memcpy(leftArray.data(), &pairs[left], n1 * sizeof(std::pair<int, int>));
	if (n2 > 0)
		__builtin_memcpy(rightArray.data(), &pairs[middle + 1], n2 * sizeof(std::pair<int, int>));
	
	while (i + 3 < n1 && j + 3 < n2) {
		merge4_unrolled(pairs, leftArray, rightArray, i, j, k, ComparePairs());
	}

	while (i < n1 && j < n2) {
		bool c    = ComparePairs()(leftArray[i], rightArray[j]);
		pairs[k]  = c ? leftArray[i] : rightArray[j];
		i        += c ? 1 : 0;
		j        += c ? 0 : 1;
		++k;
	}

	while (i + 7 < n1) {  
		_mm_prefetch(reinterpret_cast<const char*>(&leftArray[i + 8]), _MM_HINT_NTA);
		_mm_prefetch(reinterpret_cast<const char*>(&rightArray[j + 8]), _MM_HINT_NTA);

		__m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&rightArray[j]));
		__m256i data2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&leftArray[i]));
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(&pairs[k]), data);
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(&pairs[k]), data2);

		i += 8;
		k += 8;
	}

	while (i < n1) 
		pairs[k++] = leftArray[i++];

	while (j < n2) 
		pairs[k++] = rightArray[j++];

}

void sort_pairs(std::vector<std::pair<int, int> >& pairs, int left, int right) {
    const int threshold = 32;
    std::vector<std::pair<int, int> > leftBuffer((right - left + 1) / 2 + 1);
    std::vector<std::pair<int, int> > rightBuffer((right - left + 1) / 2 + 1);

    if (__builtin_expect(right - left < threshold, 1)) {
        std::sort(pairs.begin() + left, pairs.begin() + right + 1, ComparePairs());
        return;
    }

    int middle = left + ((right - left) >> 1);
    sort_pairs(pairs, left, middle);
    sort_pairs(pairs, middle + 1, right);
    merge_pairs(pairs, left, middle, right);
}

std::vector<int> ford_johnson_sort(std::vector<std::pair<int, int> >& pairs, int straggler, bool has_straggler) {
    compare_pairs_avx(pairs);
    sort_pairs(pairs, 0, pairs.size() - 1);

    size_t size = pairs.size();
    std::vector<int> S;
    std::vector<int> pend;
    S.reserve(size);
    pend.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        S.push_back(pairs[i].second);
        pend.push_back(pairs[i].first);
    }

    std::vector<uint64_t> jacobsthal_sequence = generate_jacobsthal_AVX(pend.size());
    std::vector<bool> inserted(pend.size(), false);
	#pragma omp parallel for
    for (size_t i = 0; i < jacobsthal_sequence.size(); ++i){
        size_t idx = jacobsthal_sequence[i];
        if (idx < pend.size() && !inserted[idx]) {
            insertion(S, pend[idx]);
            inserted[idx] = true;
        }
    }

    if (has_straggler)
        insertion(S, straggler);

    for (size_t i = 0; i < pend.size(); ++i) {
        if (!inserted[i]) {
            insertion(S, pend[i]);
            inserted[i] = true;
        }
    }
	std::cout << "comparison_count: " << comparison_count << std::endl;
    return S;
}
