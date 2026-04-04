#include <algorithm>
#include <list>
#include <new>
#include <ranges>

#include "allocator.h"

allocator::bad_dealloc::bad_dealloc() {}

const char* allocator::bad_dealloc::what() const noexcept {
    return "bb::bad_dealloc: invalid pointer or double free";
}

allocator::allocator() : allocator(strategy::FIRST_FIT) {}

allocator::allocator(strategy s) : strategy_{s} {}

void allocator::dealloc(void* ptr) {
    if (!ptr)
        return;

    auto* blk{(static_cast<alloc_t*>(ptr)) - 1}; // get the header block
    // could use assert
    if (blk->flag_ != flag::USED)
        throw bad_dealloc();
    if (blk->addr_ != ptr)
        throw bad_dealloc();

    blk->flag_ = flag::UNUSED; // invalidate magic
    blk_used_.remove(blk);
    blk_free_.push_back(blk);
    // coalesce(blk); // TODO
}

const char* allocator::to_string(strategy s) {
    switch (s) {
    case strategy::FIRST_FIT:
        return "first fit";
    case strategy::BEST_FIT:
        return "best fit";
    case strategy::WORST_FIT:
        return "worst fit";
    default:
        return "unknown";
    }
}