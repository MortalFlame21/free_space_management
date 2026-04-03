# Free space management

This repository is to understand how free space management works under the hood of the OS, and how it satisfies variable-sized requests. What are the strategies involved to minimize fragmentation and what are the various strengths/weaknesses of such algorithms.

This project includes a playground memory allocation functions, `T* alloc<T>()` and `void dealloc(void* ptr)` that mimics C++ keyword `new` and `delete` and the C standard functions `void* malloc(size_t sz)` and `void free(void* ptr)`. As such, `alloc` is closer C++ keyword `new` rather than C standard `malloc`. `dealloc` is closer to C standard `free` than keyword `delete`.

## Assumptions

- Instead of initialising some sort of linked list for the free list, we use `std::list<alloc_t>` as a way to track free blocks.
- To grow the heap I use POSIX `sbrk`.

## Requirements

- A C++20 compliant compiler.
- CMake v3.15 or higher.

## Usage

### `rand_allocs.cpp`

```shell
cmake -S . -B build
make -C ./build
./build/mem_alloc <NUM_ALLOCATIONS> <NUM_DEALLOCATIONS>
```

### Memory allocation function usage

_Note: The functions are not production ready._

```cpp
#include "include/bb/malloc.cpp"
int main() {
    auto* ptr{bb::alloc<int>()}; // would return a int*, or T*.
    bb::dealloc(ptr); // free
}
```
