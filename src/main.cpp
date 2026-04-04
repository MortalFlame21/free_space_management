#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <ranges>
#include <vector>

#include "bb/alloc.h"

namespace cno = std::chrono;
using clk = std::chrono::steady_clock;

void run(int num_allocs, int num_deallocs);
void print_strategy(bb::alloc_strategy s, clk::time_point t0, clk::time_point t1);

std::random_device rd{};
std::mt19937 mt{rd()};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: mem_allocations <NUM_ALLOCATIONS> <NUM_DEALLOCATIONS>";
        return EXIT_FAILURE;
    }

    // invariant: allocs >= deallocs.
    int num_deallocs{std::max(1, std::stoi(argv[2]))};
    int num_allocs{std::max({1, std::stoi(argv[1]), num_deallocs})};

    // we don't know the amount of num_allocs,
    // use try catch incase of std::bad_alloc or bb::bad_dealloc
    try {
        run(num_allocs, num_deallocs);
    } catch (std::exception& e) {
        std::cerr << "caught: " << e.what() << '\n';
        std::cerr << "\tpossibly too much allocations (" << num_allocs << ")\n";
    }
}

void run(int num_allocs, int num_deallocs) {
    namespace rng = std::ranges;
    using stgy = bb::alloc_strategy;

    for (int s{}; s < static_cast<int>(stgy::MAX); ++s) {
        std::vector<int*> allocations(num_allocs, {});
        auto strategy{static_cast<stgy>(s)};
        auto t0{clk::now()};

        for (auto& ptr : allocations) {
            ptr = bb::alloc<int>(strategy);
            *ptr = mt();
        }

        rng::shuffle(allocations, mt);

        for (int i{}; i < num_deallocs; ++i) {
            bb::dealloc(allocations[i]);
        }

        print_strategy(strategy, t0, clk::now());
    }
}

// invariant t0 < t1, we assume this
void print_strategy(bb::alloc_strategy s, clk::time_point t0, clk::time_point t1) {
    auto d{t0 - t1};
    std::cout << bb::to_string(s) << '\n'
              << '\t' << cno::duration_cast<cno::nanoseconds>(d) << '\n'
              << '\t' << cno::duration_cast<cno::microseconds>(d) << '\n'
              << '\t' << cno::duration_cast<cno::milliseconds>(d) << '\n'
              << '\t' << cno::duration_cast<cno::seconds>(d) << '\n';
}