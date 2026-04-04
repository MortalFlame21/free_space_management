#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <ranges>
#include <stack>
#include <vector>

#include "allocator.h"

namespace cno = std::chrono;
using clk = std::chrono::steady_clock;

enum class instruction { ALLOC, DEALLOC };

std::random_device rd{};
std::mt19937 mt{rd()};

void run(int num_allocs, int num_deallocs);
void run_strategy(allocator::strategy s, const std::vector<instruction>& calls);
void print_strategy(allocator::strategy s, clk::time_point t0, clk::time_point t1);

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
    using stgy = allocator::strategy;

    // being randomised we should double check for valid deallocs.
    std::vector<instruction> calls(num_allocs + num_deallocs, instruction::ALLOC);
    rng::fill_n(calls.begin(), num_deallocs, instruction::DEALLOC);
    rng::shuffle(calls, mt);

    for (int s{}; s < static_cast<int>(stgy::MAX); ++s) {
        auto strategy{static_cast<stgy>(s)};
        auto t0{clk::now()};

        run_strategy(strategy, calls);

        print_strategy(strategy, t0, clk::now());
    }
}

void run_strategy(allocator::strategy s, const std::vector<instruction>& calls) {
    std::stack<int*> alloc_st{};
    allocator a(s);
    for (auto& c : calls) {
        try {
            switch (c) {
            case instruction::ALLOC:
                alloc_st.push(a.alloc<int>());
                break;
            case instruction::DEALLOC:
                if (alloc_st.empty()) // just throw
                    throw allocator::bad_dealloc();

                a.dealloc(alloc_st.top());
                alloc_st.pop();
                break;
            default:
                break;
            }
        } catch (allocator::bad_dealloc& e) {
            std::cerr << "caught: " << e.what() << '\n'
                      << "\tlikely double free. continuing.\n";
        } catch (std::exception& e) {
            throw; // rethrow the likely std::bad_alloc
        }
    }
}

// invariant t0 < t1, we assume this
void print_strategy(allocator::strategy s, clk::time_point t0, clk::time_point t1) {
    auto d{t0 - t1};
    std::cout << allocator::to_string(s) << '\n'
              << '\t' << cno::duration_cast<cno::nanoseconds>(d) << '\n'
              << '\t' << cno::duration_cast<cno::microseconds>(d) << '\n'
              << '\t' << cno::duration_cast<cno::milliseconds>(d) << '\n'
              << '\t' << cno::duration_cast<cno::seconds>(d) << '\n';
}