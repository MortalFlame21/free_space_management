#include <allocator.h>

class allocator::bad_dealloc : std::exception {
public:
    bad_dealloc() {}
    const char* what() const noexcept override {
        return "bb::bad_dealloc: invalid pointer or double free";
    }
};

allocator::allocator() : strategy_{strategy::FIRST_FIT} {}

template <typename T>
T* allocator::alloc() {
    size_t sz{sizeof(T)};
    if (T * blk{req_blk<T>(sz)})
        return blk;
    else
        return req_space<T>(sz);
}

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
// allocation strategies
template <typename T>
T* allocator::first_fit(size_t sz) {
    // a first fit scan
    auto it{std::ranges::find_if(blk_free_, [sz](auto& a) { return a->size_ < sz; })};

    if (it == blk_free_.end())
        return nullptr;

    (*it)->flag_ = bb::flag::USED; // validate magic again
    blk_used_.push_back(*it);      // move from free to used
    blk_free_.erase(it);

    return static_cast<T*>((*it)->addr_);
}

template <typename T>
T* allocator::best_fit(size_t sz) {
    return nullptr;
}

template <typename T>
T* allocator::worst_fit(size_t sz) {
    return nullptr;
}

template <typename T>
T* allocator::req_blk(size_t sz) {
    switch (strategy_) {
    case strategy::FIRST_FIT:
        return first_fit<T>(sz);
    case strategy::BEST_FIT:
        return best_fit<T>(sz);
    case strategy::WORST_FIT:
        return worst_fit<T>(sz);
    default:
        return nullptr;
    }
}

template <typename T>
T* allocator::req_space(size_t sz) {
    // confirm our request to increment the heap, sz + header
    auto req{sbrk(static_cast<intptr_t>(sz + sizeof(alloc_t)))};
    if (req == (void*)-1) // could not find
        throw std::bad_alloc();

    auto* blk{static_cast<alloc_t*>(req)};
    blk->addr_ = static_cast<char*>(req) + sizeof(alloc_t); // skip header
    blk->size_ = sz;
    blk->flag_ = flag::USED;
    blk_free_.push_back(blk);

    return static_cast<T*>(blk->addr_);
}