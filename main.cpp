#include "PmergeMe.hpp"

#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>

static bool parse_positive_int(const char* text, int& value) {
    char* end = NULL;
    long parsed = 0;

    if (text == NULL || *text == '\0')
        return false;

    errno = 0;
    parsed = std::strtol(text, &end, 10);
    if (errno != 0 || *end != '\0' || parsed < 0 || parsed > std::numeric_limits<int>::max())
        return false;

    value = static_cast<int>(parsed);
    return true;
}

static void print_stats(const std::string& label, size_t data_count, uint64_t comparisons) {
    const long double n = static_cast<long double>(data_count);
    const long double c = static_cast<long double>(comparisons);
    const long double n_log_n = (data_count > 1) ? (n * std::log2(n)) : 0.0L;
    const long double lower_bound = (data_count > 1) ? (std::lgammal(n + 1.0L) / std::log(2.0L)) : 0.0L;
    const long double ratio_per_item = (data_count > 0) ? (c / n) : 0.0L;
    const long double normalized_n_log_n = (n_log_n > 0.0L) ? (c / n_log_n) : 0.0L;
    const long double lower_bound_ratio = (lower_bound > 0.0L) ? (c / lower_bound) : 0.0L;
    const long double empirical_exponent = (data_count > 1 && comparisons > 0)
        ? (std::log(c) / std::log(n))
        : 0.0L;

    std::cout << "Stats for " << label << ":" << std::endl;
    std::cout << "  n = " << data_count << std::endl;
    std::cout << "  comparisons = " << comparisons << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  comparisons / n = " << ratio_per_item << std::endl;
    std::cout << "  comparisons / (n log2 n) = " << normalized_n_log_n << std::endl;
    std::cout << "  comparisons / log2(n!) = " << lower_bound_ratio << std::endl;
    std::cout << "  empirical exponent p in n^p = " << empirical_exponent << std::endl;
    std::cout << std::defaultfloat;
}

int main(int argc, char* argv[]) {
    std::vector<int> data;
    std::deque<int> data_deque;
    bool show_stats = false;

    if (argc <= 1) {
        std::cout << "Usage: " << argv[0] << " [--stats] <number1> <number2> ... <numberN>" << std::endl;
        std::cout << "Example: " << argv[0] << " --stats 5 8 1 3 9 2" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        int parsed = 0;

        if (std::string(argv[i]) == "--stats" || std::string(argv[i]) == "-s") {
            show_stats = true;
            continue;
        }

        if (!parse_positive_int(argv[i], parsed)) {
            std::cerr << "Invalid argument: " << argv[i] << std::endl;
            return 1;
        }

        data.push_back(parsed);
        data_deque.push_back(parsed);
    }

    if (data.empty()) {
        std::cerr << "No numbers to sort" << std::endl;
        return 1;
    }

    const size_t total_input_count = data.size();

    int straggler = -1;
    bool has_straggler = false;
    int n = static_cast<int>(data.size());

    if ((n % 2) != 0) {
        has_straggler = true;
        straggler = data.back();
        data.pop_back();
        data_deque.pop_back();
        --n;
    }

    const std::chrono::high_resolution_clock::time_point start_vector = std::chrono::high_resolution_clock::now();
    const std::vector<int> sorted_vector = ford_johnson_sort(data, straggler, has_straggler);
    const std::chrono::high_resolution_clock::time_point end_vector = std::chrono::high_resolution_clock::now();
    const uint64_t vector_comparisons = get_comparison_count();

    const std::chrono::high_resolution_clock::time_point start_deque = std::chrono::high_resolution_clock::now();
    const std::deque<int> sorted_deque = ford_johnson_sort_deque(data_deque, straggler, has_straggler);
    const std::chrono::high_resolution_clock::time_point end_deque = std::chrono::high_resolution_clock::now();
    const uint64_t deque_comparisons = get_comparison_count();

    if (!std::is_sorted(sorted_vector.begin(), sorted_vector.end())) {
        std::cerr << "Vector result is not sorted" << std::endl;
        return 1;
    }

    if (!std::is_sorted(sorted_deque.begin(), sorted_deque.end())) {
        std::cerr << "Deque result is not sorted" << std::endl;
        return 1;
    }

    if (sorted_vector.size() != sorted_deque.size() ||
        !std::equal(sorted_vector.begin(), sorted_vector.end(), sorted_deque.begin())) {
        std::cerr << "Vector and deque results differ" << std::endl;
        return 1;
    }

    const double vector_time = std::chrono::duration_cast<std::chrono::microseconds>(end_vector - start_vector).count();
    const double deque_time = std::chrono::duration_cast<std::chrono::microseconds>(end_deque - start_deque).count();

    // std::cout << "Sorted numbers: ";
    // for (size_t i = 0; i < sorted_vector.size(); ++i)
    //     std::cout << sorted_vector[i] << " ";
    // std::cout << std::endl;

    std::cout << "Time for vector sort: " << vector_time << " us" << std::endl;
    std::cout << "Time for deque sort: " << deque_time << " us" << std::endl;

    if (show_stats) {
        std::cout << "Sort mode: " << get_sort_mode_name() << std::endl;
        print_stats("vector", total_input_count, vector_comparisons);
        print_stats("deque", total_input_count, deque_comparisons);
    }

    return 0;
}
