#include <list>
#include <new>

#include <unistd.h>

class allocator {
private:
    enum class flag { UNUSED, USED };

    // a header block
    struct alloc_t {
        void* addr_{};
        size_t size_{};
        flag flag_{};
    };

public:
    enum class strategy { FIRST_FIT, BEST_FIT, WORST_FIT, MAX };

    // a copy of std::bad_alloc
    class bad_dealloc : public std::exception {
    public:
        bad_dealloc();
        const char* what() const noexcept override;
    };

    allocator();
    allocator(strategy s);

    template <typename T>
    T* alloc();
    void dealloc(void* ptr);

    // other helper
    static const char* to_string(strategy s);

private:
    std::list<alloc_t*> blk_used_{};
    std::list<alloc_t*> blk_free_{};
    strategy strategy_{strategy::FIRST_FIT};
    inline static std::list<alloc_t*> s_blk_free_{};

    // allocation strategies
    template <typename T>
    T* first_fit(size_t sz);
    template <typename T>
    T* worst_fit(size_t sz);
    template <typename T>
    T* best_fit(size_t sz);

    // auxilary
    template <typename T>
    T* req_blk(size_t sz);
    template <typename T>
    T* req_space(size_t sz);
};

// templates
// core
template <typename T>
T* allocator::alloc() {
    size_t sz{sizeof(T)};
    if (T * blk{req_blk<T>(sz)})
        return blk;
    else
        return req_space<T>(sz);
}

// allocation strategies
template <typename T>
T* allocator::first_fit(size_t sz) {
    // a first fit scan
    auto it{std::ranges::find_if(blk_free_, [sz](auto& a) { return a->size_ < sz; })};

    if (it == blk_free_.end())
        return nullptr;

    (*it)->flag_ = flag::USED; // validate magic again
    blk_used_.push_back(*it);  // move from free to used
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