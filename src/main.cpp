#include <algorithm>
#include <iostream>
#include <random>
#include <ranges>
#include <vector>

#include "bb/alloc.h"

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
    std::vector<int*> g_allocations(num_allocs, {});

    // we don't know the amount of num_allocs, use try catch incase of std::bad_alloc or
    // bb::bad_dealloc
    try {
        namespace rng = std::ranges;
        using stgy = bb::alloc_strategy;

        for (int s{}; s < static_cast<int>(stgy::MAX); ++s) {
            auto strategy{static_cast<stgy>(s)};

            // time start

            for (auto& ptr : g_allocations) {
                ptr = bb::alloc<int>(strategy);
                *ptr = mt();
            }

            rng::shuffle(g_allocations, mt);

            for (int i{}; i < num_deallocs; ++i) {
                bb::dealloc(g_allocations[i]);
            }

            // time end
        }
    } catch (std::exception& e) {
        std::cerr << "caught: " << e.what() << '\n';
        std::cerr << "\tpossibly too much allocations (" << num_allocs << ")\n";
    }
}
