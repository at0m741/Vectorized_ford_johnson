#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <deque>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <list of numbers>" << std::endl;
        return 1;
    }

    std::vector<int> unsorted_list;
    for (int i = 1; i < argc; ++i) {
        unsorted_list.push_back(std::atoi(argv[i]));
    }
    
    std::deque<int> unsorted_list2;
    for (int i = 1; i < argc; ++i) {
        unsorted_list2.push_back(std::atoi(argv[i]));
    }

    // Mesurer le temps de std::sort sur std::vector
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(unsorted_list.begin(), unsorted_list.end());
    auto end = std::chrono::high_resolution_clock::now();

    double time1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Sorting (vector) std::sort completed in " << time1 << " µs.\n";

    return 0;
}
