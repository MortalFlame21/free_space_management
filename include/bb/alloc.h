#include <algorithm>
#include <list>
#include <new>
#include <ranges>

#include <unistd.h>

namespace bb {
// enums, structs/classes and constants
enum class flag { UNUSED, USED };
enum class alloc_strategy { FIRST_FIT, BEST_FIT, WORST_FIT, MAX };

// a header block
struct alloc_t {
    void* addr_{};
    size_t size_{};
    flag flag_{};
};

// a copy of std::bad_alloc
class bad_dealloc : std::exception {
public:
    bad_dealloc() {}
    const char* what() const noexcept override {
        return "bb::bad_dealloc: invalid pointer or double free";
    }
};

static std::list<alloc_t*> blk_used{};
static std::list<alloc_t*> blk_free{};

// allocation strategies
template <typename T>
static T* first_fit(size_t sz) {
    // a first fit scan
    auto it{std::ranges::find_if(blk_free, [sz](auto& a) { return a->size_ < sz; })};

    if (it == blk_free.end())
        return nullptr;

    (*it)->flag_ = bb::flag::USED; // validate magic again
    blk_used.push_back(*it);       // move from free to used
    blk_free.erase(it);

    return static_cast<T*>((*it)->addr_);
}

template <typename T>
static T* best_fit(size_t sz) {
    return nullptr;
}

template <typename T>
static T* worst_fit(size_t sz) {
    return nullptr;
}

// allocation and deallocation auxilary
template <typename T>
static T* req_blk(size_t sz, bb::alloc_strategy stgy = bb::alloc_strategy::FIRST_FIT) {
    switch (stgy) {
    case bb::alloc_strategy::FIRST_FIT:
        return first_fit<T>(sz);
    case bb::alloc_strategy::BEST_FIT:
        return best_fit<T>(sz);
    case bb::alloc_strategy::WORST_FIT:
        return worst_fit<T>(sz);
    default:
        return nullptr;
    }
}

template <typename T>
static T* req_space(size_t sz) {
    // confirm our request to increment the heap, sz + header
    auto req{sbrk(static_cast<intptr_t>(sz + sizeof(alloc_t)))};
    if (req == (void*)-1) // could not find
        throw std::bad_alloc();

    auto* blk{static_cast<alloc_t*>(req)};
    blk->addr_ = static_cast<char*>(req) + sizeof(alloc_t); // skip header
    blk->size_ = sz;
    blk->flag_ = bb::flag::USED;
    blk_free.push_back(blk);

    return static_cast<T*>(blk->addr_);
}

// allocation and deallocation core
template <typename T>
T* alloc(bb::alloc_strategy stgy = bb::alloc_strategy::FIRST_FIT) {
    size_t sz{sizeof(T)};
    if (T * blk{req_blk<T>(sz, stgy)})
        return blk;
    else
        return req_space<T>(sz);
}

void dealloc(void* ptr) {
    if (!ptr)
        return;

    auto* blk{(static_cast<alloc_t*>(ptr)) - 1}; // get the header block
    // could use assert
    if (blk->flag_ != bb::flag::USED)
        throw bb::bad_dealloc();
    if (blk->addr_ != ptr)
        throw bb::bad_dealloc();

    blk->flag_ = bb::flag::UNUSED; // invalidate magic
    blk_used.remove(blk);
    blk_free.push_back(blk);
    // coalesce(blk); // TODO
}
} // namespace bb