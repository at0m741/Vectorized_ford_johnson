#include "PmergeMe.hpp"

extern uint64_t comparison_count;

static inline void normalize_pair_scalar(int first, int second, int &lower,
                                         int &upper) {
  lower = std::min(first, second);
  upper = std::max(first, second);
}

PairSoA build_pair_soa(const std::vector<int> &values) {
  const size_t pair_count = values.size() / 2;
  PairSoA pairs(pair_count);
  size_t pair_index = 0;
  size_t value_index = 0;

#if PMERGEME_SIMD_NEON

#if PMERGEME_SIMD_UNROLL

  for (; value_index + 16 <= values.size();
       value_index += 16, pair_index += 8) {
    const int32x4x2_t interleaved0 = vld2q_s32(values.data() + value_index);
    const int32x4x2_t interleaved1 = vld2q_s32(values.data() + value_index + 8);

    const int32x4_t lower0 =
        vminq_s32(interleaved0.val[0], interleaved0.val[1]);
    const int32x4_t upper0 =
        vmaxq_s32(interleaved0.val[0], interleaved0.val[1]);

    const int32x4_t lower1 =
        vminq_s32(interleaved1.val[0], interleaved1.val[1]);
    const int32x4_t upper1 =
        vmaxq_s32(interleaved1.val[0], interleaved1.val[1]);

    vst1q_s32(pairs.lower.data() + pair_index, lower0);
    vst1q_s32(pairs.upper.data() + pair_index, upper0);

    vst1q_s32(pairs.lower.data() + pair_index + 4, lower1);
    vst1q_s32(pairs.upper.data() + pair_index + 4, upper1);

    comparison_count += 8;
  }

#endif

  for (; value_index + 8 <= values.size(); value_index += 8, pair_index += 4) {
    const int32x4x2_t interleaved = vld2q_s32(values.data() + value_index);
    const int32x4_t lower = vminq_s32(interleaved.val[0], interleaved.val[1]);
    const int32x4_t upper = vmaxq_s32(interleaved.val[0], interleaved.val[1]);

    vst1q_s32(pairs.lower.data() + pair_index, lower);
    vst1q_s32(pairs.upper.data() + pair_index, upper);

    comparison_count += 4;
  }

#endif

  for (; pair_index < pair_count; ++pair_index, value_index += 2) {
    normalize_pair_scalar(values[value_index], values[value_index + 1],
                          pairs.lower[pair_index], pairs.upper[pair_index]);
    ++comparison_count;
  }

  return pairs;
}

PairSoA build_pair_soa(const std::deque<int> &values) {
  const size_t pair_count = values.size() / 2;
  PairSoA pairs(pair_count);

  for (size_t pair_index = 0, value_index = 0; pair_index < pair_count;
       ++pair_index, value_index += 2) {
    normalize_pair_scalar(values[value_index], values[value_index + 1],
                          pairs.lower[pair_index], pairs.upper[pair_index]);
    ++comparison_count;
  }

  return pairs;
}

size_t find_insertion_position(const std::vector<int> &arr, size_t size,
                               int value) {
  size_t i = 0;

#if PMERGEME_SIMD_AVX2

  const __m256i vector_value = _mm256_set1_epi32(value);

  for (; i + 8 <= size; i += 8) {
    const __m256i data =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(arr.data() + i));
    const __m256i gt = _mm256_cmpgt_epi32(data, vector_value);
    const __m256i eq = _mm256_cmpeq_epi32(data, vector_value);
    const __m256i ge = _mm256_or_si256(gt, eq);
    const int mask = _mm256_movemask_ps(_mm256_castsi256_ps(ge));

    comparison_count += 8;
    if (mask != 0)
      return i + static_cast<size_t>(
                     __builtin_ctz(static_cast<unsigned int>(mask)));
  }
#elif PMERGEME_SIMD_NEON
  const int32x4_t vector_value = vdupq_n_s32(value);
  const uint32x4_t weights = {1u, 2u, 4u, 8u};

  for (; i + 4 <= size; i += 4) {
    const int32x4_t data = vld1q_s32(arr.data() + i);
    const uint32x4_t ge = vcgeq_s32(data, vector_value);
    const uint32x4_t bits = vandq_u32(ge, vdupq_n_u32(1u));
    const uint32x4_t weighted = vmulq_u32(bits, weights);
    const unsigned int mask = vaddvq_u32(weighted);

    comparison_count += 4;

    if (mask != 0)
      return i + static_cast<size_t>(__builtin_ctz(mask));
  }
#endif
  for (; i < size; ++i) {
    ++comparison_count;
    if (arr[i] >= value)
      return i;
  }

  return size;
}
