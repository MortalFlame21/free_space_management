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

    // we don't know the amount of
    try {
        namespace rng = std::ranges;

        rng::generate(g_allocations, [] { return bb::alloc<int>(); });
        rng::for_each_n(g_allocations.begin(), num_allocs,
                        [&](auto ptr) { *ptr = mt(); });

        rng::shuffle(g_allocations, mt);

        rng::for_each_n(g_allocations.begin(), num_deallocs,
                        [](auto ptr) { bb::dealloc(ptr); });
    } catch (std::exception& e) {
        std::cerr << "caught: " << e.what() << '\n';
        std::cerr << "\tpossibly too much allocations (" << num_allocs << ")\n";
    }
}
