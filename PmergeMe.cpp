#include "PmergeMe.hpp"

#include <cstring>

uint64_t comparison_count = 0;

namespace {

#if defined(PMERGEME_MODE_FAST)

static const size_t kBlockTargetSize = 96;
static const size_t kBlockMaxSize = 160;

class SortedBlockChain {
public:
    SortedBlockChain() : size_(0) {
    }

    explicit SortedBlockChain(const std::vector<int>& sorted_values) : size_(0) {
        build(sorted_values);
    }

    void build(const std::vector<int>& sorted_values) {
        blocks_.clear();
        size_ = sorted_values.size();

        if (sorted_values.empty())
            return;

        for (size_t offset = 0; offset < sorted_values.size(); offset += kBlockTargetSize) {
            const size_t end = std::min(offset + kBlockTargetSize, sorted_values.size());
            std::vector<int> block;

            block.reserve(kBlockMaxSize);
            block.insert(block.end(), sorted_values.begin() + offset, sorted_values.begin() + end);
            blocks_.push_back(block);
        }
    }

    void insert(int value) {
        if (blocks_.empty()) {
            std::vector<int> block;

            block.reserve(kBlockMaxSize);
            block.push_back(value);
            blocks_.push_back(block);
            size_ = 1;
            return;
        }

        const size_t block_index = find_block(value);
        std::vector<int>& block = blocks_[block_index];
        const size_t position = find_insertion_position(block, block.size(), value);
        const size_t old_size = block.size();

        block.push_back(value);
        if (position < old_size)
            std::memmove(block.data() + position + 1, block.data() + position, (old_size - position) * sizeof(int));

        block[position] = value;
        ++size_;

        if (block.size() > kBlockMaxSize)
            split_block(block_index);
    }

    std::vector<int> flatten() const {
        std::vector<int> values;

        values.reserve(size_);
        for (size_t block_index = 0; block_index < blocks_.size(); ++block_index)
            values.insert(values.end(), blocks_[block_index].begin(), blocks_[block_index].end());

        return values;
    }

private:
    size_t find_block(int value) const {
        size_t left = 0;
        size_t right = blocks_.size();

        while (left < right) {
            const size_t middle = left + ((right - left) / 2);

            ++comparison_count;
            if (blocks_[middle].back() < value)
                left = middle + 1;
            else
                right = middle;
        }

        if (left == blocks_.size())
            return blocks_.size() - 1;

        return left;
    }

    void split_block(size_t block_index) {
        std::vector<int>& block = blocks_[block_index];
        const size_t middle = block.size() / 2;
        std::vector<int> tail;

        tail.reserve(kBlockMaxSize);
        tail.insert(tail.end(), block.begin() + middle, block.end());
        block.resize(middle);
        blocks_.insert(blocks_.begin() + block_index + 1, tail);
    }

    std::vector<std::vector<int> > blocks_;
    size_t size_;
};

#elif defined(PMERGEME_MODE_MINCMP)

class FenwickShifts {
public:
    explicit FenwickShifts(size_t size)
        : tree_(size + 1, 0) {
    }

    void add_suffix(size_t start, int delta) {
        if (start >= size())
            return;

        for (size_t index = start + 1; index < tree_.size(); index += lowbit(index))
            tree_[index] += delta;
    }

    int point_query(size_t index) const {
        int sum = 0;

        for (size_t current = index + 1; current > 0; current -= lowbit(current))
            sum += tree_[current];

        return sum;
    }

    size_t size() const {
        return tree_.size() - 1;
    }

private:
    static size_t lowbit(size_t index) {
        return index & (~index + 1);
    }

    std::vector<int> tree_;
};

#endif

}

static inline bool pair_less(const PairSoA& pairs, size_t lhs, size_t rhs) {
    ++comparison_count;
    if (pairs.upper[lhs] < pairs.upper[rhs])
        return true;

    ++comparison_count;
    if (pairs.upper[rhs] < pairs.upper[lhs])
        return false;

    ++comparison_count;
    return pairs.lower[lhs] < pairs.lower[rhs];
}

static void insertion_sort_pairs(PairSoA& pairs, size_t left, size_t right) {
    for (size_t i = left + 1; i <= right; ++i) {
        int current_lower = pairs.lower[i];
        int current_upper = pairs.upper[i];
        size_t j = i;

        while (j > left) {
            size_t previous = j - 1;

            ++comparison_count;
            if (pairs.upper[previous] < current_upper)
                break;

            ++comparison_count;
            if (pairs.upper[previous] == current_upper) {
                ++comparison_count;
                if (pairs.lower[previous] <= current_lower)
                    break;
            }

            pairs.lower[j] = pairs.lower[previous];
            pairs.upper[j] = pairs.upper[previous];
            --j;
        }

        pairs.lower[j] = current_lower;
        pairs.upper[j] = current_upper;
    }
}

static void merge_pairs_by_upper(PairSoA& pairs, PairSoA& buffer, size_t left, size_t middle, size_t right) {
    size_t lhs = left;
    size_t rhs = middle + 1;
    size_t out = left;

    while (lhs <= middle && rhs <= right) {
        if (pair_less(pairs, rhs, lhs)) {
            buffer.lower[out] = pairs.lower[rhs];
            buffer.upper[out] = pairs.upper[rhs];
            ++rhs;
        } else {
            buffer.lower[out] = pairs.lower[lhs];
            buffer.upper[out] = pairs.upper[lhs];
            ++lhs;
        }
        ++out;
    }

    while (lhs <= middle) {
        buffer.lower[out] = pairs.lower[lhs];
        buffer.upper[out] = pairs.upper[lhs];
        ++lhs;
        ++out;
    }

    while (rhs <= right) {
        buffer.lower[out] = pairs.lower[rhs];
        buffer.upper[out] = pairs.upper[rhs];
        ++rhs;
        ++out;
    }

    for (size_t index = left; index <= right; ++index) {
        pairs.lower[index] = buffer.lower[index];
        pairs.upper[index] = buffer.upper[index];
    }
}

static void merge_sort_pairs_by_upper(PairSoA& pairs, PairSoA& buffer, size_t left, size_t right) {
    static const size_t threshold = 32;

    if (left >= right)
        return;

    if (right - left < threshold) {
        insertion_sort_pairs(pairs, left, right);
        return;
    }

    const size_t middle = left + ((right - left) / 2);
    merge_sort_pairs_by_upper(pairs, buffer, left, middle);
    merge_sort_pairs_by_upper(pairs, buffer, middle + 1, right);
    merge_pairs_by_upper(pairs, buffer, left, middle, right);
}

static void sort_pairs_by_upper(PairSoA& pairs) {
    if (pairs.size() < 2)
        return;

    PairSoA buffer(pairs.size());
    merge_sort_pairs_by_upper(pairs, buffer, 0, pairs.size() - 1);
}

static std::vector<size_t> build_jacobsthal_insert_order(size_t pair_count) {
    std::vector<size_t> order;

    if (pair_count <= 1)
        return order;

    const std::vector<uint64_t> jacobsthal = generate_jacobsthal_sequence(pair_count + 2);
    size_t previous = 1;

    order.reserve(pair_count - 1);
    for (size_t jacob_index = 3; previous < pair_count; ++jacob_index) {
        const size_t current = static_cast<size_t>(std::min<uint64_t>(jacobsthal[jacob_index], pair_count));

        for (size_t index = current; index > previous; --index)
            order.push_back(index - 1);

        if (current == pair_count)
            break;

        previous = static_cast<size_t>(jacobsthal[jacob_index]);
    }

    return order;
}

#if defined(PMERGEME_MODE_MINCMP)
static void insert_into_chain(std::vector<int>& chain, size_t position, int value) {
    const size_t old_size = chain.size();

    chain.push_back(value);
    if (position < old_size)
        std::memmove(chain.data() + position + 1, chain.data() + position, (old_size - position) * sizeof(int));

    chain[position] = value;
}

static size_t find_insertion_position_binary(const std::vector<int>& chain, size_t size, int value) {
    size_t left = 0;
    size_t right = size;

    while (left < right) {
        const size_t middle = left + ((right - left) / 2);

        ++comparison_count;
        if (chain[middle] < value)
            left = middle + 1;
        else
            right = middle;
    }

    return left;
}

static size_t current_upper_position(const FenwickShifts& shifts, size_t upper_index) {
    return upper_index + 1 + static_cast<size_t>(shifts.point_query(upper_index));
}

static size_t find_first_affected_upper(const FenwickShifts& shifts, size_t position) {
    size_t left = 0;
    size_t right = shifts.size();

    while (left < right) {
        const size_t middle = left + ((right - left) / 2);

        if (current_upper_position(shifts, middle) < position)
            left = middle + 1;
        else
            right = middle;
    }

    return left;
}

static void insert_partner_low(std::vector<int>& chain,
                               FenwickShifts& shifts,
                               size_t pair_index,
                               int value) {
    const size_t bound = current_upper_position(shifts, pair_index);
    const size_t position = find_insertion_position_binary(chain, bound, value);

    insert_into_chain(chain, position, value);
    shifts.add_suffix(find_first_affected_upper(shifts, position), 1);
}
#endif

std::vector<int> ford_johnson_sort_soa(PairSoA pairs, int straggler, bool has_straggler) {
    if (pairs.empty()) {
        std::vector<int> chain;

        if (has_straggler)
            chain.push_back(straggler);
        return chain;
    }

    sort_pairs_by_upper(pairs);

    const size_t pair_count = pairs.size();
    std::vector<int> initial_chain;

    initial_chain.reserve((pair_count + 1) + (has_straggler ? 1u : 0u));
    initial_chain.push_back(pairs.lower[0]);
    for (size_t index = 0; index < pair_count; ++index)
        initial_chain.push_back(pairs.upper[index]);

#if defined(PMERGEME_MODE_FAST)
    SortedBlockChain chain(initial_chain);
#elif defined(PMERGEME_MODE_MINCMP)
    std::vector<int> chain = initial_chain;
    FenwickShifts shifts(pair_count);
#endif

    const std::vector<size_t> insertion_order = build_jacobsthal_insert_order(pair_count);
    for (size_t order_index = 0; order_index < insertion_order.size(); ++order_index) {
        const size_t pair_index = insertion_order[order_index];

#if defined(PMERGEME_MODE_FAST)
        chain.insert(pairs.lower[pair_index]);
#elif defined(PMERGEME_MODE_MINCMP)
        insert_partner_low(chain, shifts, pair_index, pairs.lower[pair_index]);
#endif
    }

#if defined(PMERGEME_MODE_FAST)
    if (has_straggler)
        chain.insert(straggler);
#elif defined(PMERGEME_MODE_MINCMP)
    if (has_straggler) {
        const size_t position = find_insertion_position_binary(chain, chain.size(), straggler);
        insert_into_chain(chain, position, straggler);
    }
#endif

#ifdef DEBUG
    std::cout << "comparison_count: " << comparison_count << std::endl;
#endif

#if defined(PMERGEME_MODE_FAST)
    return chain.flatten();
#elif defined(PMERGEME_MODE_MINCMP)
    return chain;
#endif
}

std::vector<int> ford_johnson_sort(const std::vector<int>& values, int straggler, bool has_straggler) {
    reset_comparison_count();
    return ford_johnson_sort_soa(build_pair_soa(values), straggler, has_straggler);
}

void reset_comparison_count() {
    comparison_count = 0;
}

uint64_t get_comparison_count() {
    return comparison_count;
}

const char* get_sort_mode_name() {
#if defined(PMERGEME_MODE_FAST)
    return "fast";
#elif defined(PMERGEME_MODE_MINCMP)
    return "mincmp";
#else
    return "unknown";
#endif
}
