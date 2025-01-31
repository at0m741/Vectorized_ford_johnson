#include "PmergeMe.hpp"
#include <cstring>


extern int comparison_count;
void compare_pairs_avx(std::vector<std::pair<int, int> >& pairs) {
	size_t n = pairs.size();
	size_t block_size = 256 / sizeof(std::pair<int, int>);

	for (size_t block_start = 0; block_start < n; block_start += block_size) {
		size_t block_end = std::min(block_start + block_size, n);
		size_t i = block_start;

		for (; i + 8 <= block_end; i += 8) {
			__m256i first = _mm256_set_epi32(
					pairs[i + 7].first, pairs[i + 6].first, pairs[i + 5].first, pairs[i + 4].first,
					pairs[i + 3].first, pairs[i + 2].first, pairs[i + 1].first, pairs[i + 0].first);
			__m256i second = _mm256_set_epi32(
					pairs[i + 7].second, pairs[i + 6].second, pairs[i + 5].second, pairs[i + 4].second,
					pairs[i + 3].second, pairs[i + 2].second, pairs[i + 1].second, pairs[i + 0].second);

			__m256i min_values = _mm256_min_epi32(first, second);
			__m256i max_values = _mm256_max_epi32(first, second);
			comparison_count += 8;
			int results_min[8], results_max[8];
			_mm256_storeu_si256((__m256i*)results_min, min_values);
			_mm256_storeu_si256((__m256i*)results_max, max_values);

			for (int j = 0; j < 8; ++j) {
				pairs[i + j].first = results_min[j];
				pairs[i + j].second = results_max[j];
			}
		}

		for (; i < block_end; ++i) {
			comparison_count++;
			int min_value = std::min(pairs[i].first, pairs[i].second);
            int max_value = std::max(pairs[i].first, pairs[i].second);
            pairs[i].first = min_value;
            pairs[i].second = max_value;
        }
    }
}



__attribute__((always_inline, hot))
void insertion(std::vector<int>& arr, int value) {
    arr.resize(arr.size() + 1); 

    __m256i v_val = _mm256_set1_epi32(value);
    size_t pos = arr.size() - 1; 
	__builtin_prefetch(&arr[0], 0, 3); 
    for(size_t i = 0; i + 8 <= arr.size() - 1; i += 8) {
        __m256i data = _mm256_loadu_si256((__m256i*)&arr[i]);
        __m256i cmp = _mm256_cmpgt_epi32(data, v_val);
        int mask = _mm256_movemask_epi8(cmp);
        if(mask) {
            pos = i + (_tzcnt_u32(mask) >> 2);
            break;
        }
    }

    const size_t move_size = arr.size() - 1 - pos;
    if(move_size > 0) {
		_mm_prefetch(&arr[pos], _MM_HINT_T0);
        __builtin_memmove(&arr[pos + 1], &arr[pos], move_size * sizeof(int));
    }
    arr[pos] = value;
}
