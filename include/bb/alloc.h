#include <algorithm>
#include <ranges>
#include <list>
#include <new>

#include <unistd.h>

namespace bb {
enum class flag { UNUSED, USED };

// a header block
struct alloc_t {
    void* addr_{};
    size_t size_{};
    flag flag_{};
};

class bad_dealloc : std::exception {
public:
    bad_dealloc() { }
    const char* what() const noexcept override {
        return "bb::bad_dealloc: invalid pointer or double free";
    }
};

static std::list<alloc_t*> blk_used{};
static std::list<alloc_t*> blk_free{};

template<typename T>
static T* req_blk(size_t sz) {
    // a first fit scan
    auto it{std::ranges::find_if(blk_free, [sz](auto& a){ return a->size_ < sz; })};

    if (it == blk_free.end())
        return nullptr;

    (*it)->flag_ = bb::flag::USED; // validate magic again
    blk_used.push_back(*it); // move from free to used
    blk_free.erase(it);

    return static_cast<T*>((*it)->addr_);
}

template<typename T>
static T* req_space(size_t sz) {
    // confirm our request to increment the heap, sz + header
    auto req{sbrk(static_cast<intptr_t>(sz + sizeof(alloc_t)))};
    if (req == (void*) -1) // could not find
        throw std::bad_alloc();

    auto* blk{static_cast<alloc_t*>(req)};
    blk->addr_ = static_cast<char*>(req) + sizeof(alloc_t); // skip header
    blk->size_ = sz;
    blk->flag_ = bb::flag::USED;
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

    auto* blk{(static_cast<alloc_t*>(ptr)) - 1}; // get the header block
    // could use assert
    if (blk->flag_ != bb::flag::USED)
        throw bb::bad_dealloc();
    if (blk->addr_ != ptr)
        throw bb::bad_dealloc();

    blk->flag_ = bb::flag::UNUSED; // invalidate magic
    blk_used.remove(blk);
    blk_free.push_back(blk);
}
} // namespace bb