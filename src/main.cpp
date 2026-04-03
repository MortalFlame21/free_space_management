#include <iostream>

#include "bb/alloc.h"

int main() {
    auto* ptr{bb::alloc<int>()};
    *ptr = 4;
    std::cout << *ptr << '\n';
    bb::dealloc(ptr);
}