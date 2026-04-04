# Free space management

This repository is to understand how free space management works under the hood of the OS, and how it satisfies variable-sized requests. Questions I attempt to answer are: What are the strategies involved to minimize fragmentation? What are the various strengths/weaknesses of such algorithms.

This project includes a playground memory allocation functions, `T* alloc<T>()` and `void dealloc(void* ptr)` that mimics C++ keyword `new` and `delete` and the C standard functions `void* malloc(size_t sz)` and `void free(void* ptr)`. As such, `alloc` is closer C++ keyword `new` rather than C standard `malloc`. `dealloc` is closer to C standard `free` than keyword `delete`.

## Assumptions

- To give the varying strategies a fair measurement in time, we simply use a class,
  `allocator`, which simply allows to reset the internal free list once destroyed. This allows for the strategy to start with a new free list. The destructor then places all left over free blocks to the global free list.
- Instead of initialising some sort of linked list for the free list, we use `std::list<alloc_t>` as a way to track free blocks.
- Coalescing of free blocks is not possible yet.
- To grow the heap I use UNIX `sbrk`.

## Requirements

- UNIX-like OS.
- A C++20 compliant compiler.
- CMake v3.15 or higher.

## Usage

### `main.cpp`

```shell
cmake -S . -B build
make -C ./build
./build/memory_allocation <NUM_ALLOCATIONS> <NUM_DEALLOCATIONS>
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

## Limitations

- Using `allocator` class after each strategy and having the assumption of having no free blocks in the free list per strategy, it is possible that there may not be any memory left over for another strategy to be used as all the free memory is within the global free list.
