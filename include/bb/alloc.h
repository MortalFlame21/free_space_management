#include <algorithm>
#include <ranges>
#include <list>

#include <unistd.h>

namespace bb {
// a header block
struct alloc_t {
    void* addr_{};
    size_t size_{};
    int magic_{};
};

static constexpr int MAGIC_V{69420};
static std::list<alloc_t*> blk_used{};
static std::list<alloc_t*> blk_free{};

template<typename T>
static T* req_blk(size_t sz) {
    // a first fit scan
    auto it{std::ranges::find_if(blk_free, [sz](auto& a){ return a->size_ < sz; })};

    if (it == blk_free.end())
        return nullptr;

    (*it)->magic_ = bb::MAGIC_V; // validate magic again
    blk_used.push_back(*it); // move from free to used
    blk_free.erase(it);

    return static_cast<T*>((*it)->addr_);
}

template<typename T>
static T* req_space(size_t sz) {
    // confirm our request to increment the heap
    // sz + header
    auto req = sbrk((intptr_t) (sz + sizeof(alloc_t)));
    if (req == (void*) -1)
        return nullptr;

    auto* blk = (alloc_t*) req;
    blk->addr_ = req + sizeof(alloc_t); // skip header
    blk->size_ = sz;
    blk->magic_ = MAGIC_V;

    blk_free.push_back(blk);

    return static_cast<T*>(blk->addr_);
}

template<typename T>
T* alloc() {
    constexpr size_t sz{sizeof(T)};
    if (T* blk{req_blk<T>(sz)})
        return blk;
    else
        return req_space<T>(sz);
}

void dealloc(void* ptr) {
    if (!ptr)
        return;
}
} // namespace bb