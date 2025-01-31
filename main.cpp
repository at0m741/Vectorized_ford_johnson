#include "PmergeMe.hpp"
#include <chrono>

int main(int argc, char* argv[]) 
{
    std::vector<int> data;
    std::deque<int> data2;

    if (argc <= 2 || !std::isdigit(argv[1][0]) || !std::isdigit(argv[2][0])){
        std::cout << "Usage: " << argv[0] << " <number1> <number2> ... <numberN>" << std::endl;
        std::cout << " " << std::endl;
        std::cout << "Example: " << argv[0] << " `shuf -i 1-N -n N | tr \"\\n\" \" \"`" << std::endl; 
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        std::string argument(argv[i]);
        if (argument.empty()) {
            std::cout << "Invalid argument: " << argument << std::endl;
            return 0;
        }
        data.push_back(std::atoi(argument.c_str()));
        data2.push_back(std::atoi(argument.c_str()));
    }

    if (data.empty() || data2.empty()) {
        std::cout << "Nothing to sort" << std::endl;
        return 0;
    }

    int n = data.size();
    int straggler = -1;
    bool has_straggler = false;

    if (n % 2 != 0) {
        has_straggler = true;
        straggler = data.back();
        data.pop_back();
        data2.pop_back();
        n--;
    }

    std::vector<std::pair<int, int>> pairs;
    pairs.reserve(n / 2);
    for (int i = 0; i < n; i += 2)
        pairs.push_back(std::make_pair(data[i], data[i + 1]));

    std::deque<std::pair<int, int>> deque_pairs;
    for (int i = 0; i < n; i += 2)
        deque_pairs.push_back(std::make_pair(data2[i], data2[i + 1]));

    // Remplacement de clock() par chrono
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> sorted_data = ford_johnson_sort(pairs, straggler, has_straggler);
    auto end = std::chrono::high_resolution_clock::now();

    double time1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	std::cout << "Sorted numbers: ";
    for (int num : sorted_data) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    check_if_sorted(sorted_data);

    std::cout << "Time for first sort (vector): " << time1 << " µs" << std::endl;

    return 0;
}
