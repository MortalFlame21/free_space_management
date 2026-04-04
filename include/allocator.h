#include <algorithm>
#include <list>
#include <new>
#include <ranges>

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
    class bad_dealloc;

    allocator::allocator();

    template <typename T>
    T* alloc();
    void dealloc(void* ptr);

    // other helper
    const char* to_string(strategy s);

private:
    std::list<alloc_t*> blk_used_{};
    std::list<alloc_t*> blk_free_{};
    strategy strategy_{strategy::FIRST_FIT};
    inline static std::list<alloc_t*> blk_free_{};

    // allocation strategies
    template <typename T>
    static T* first_fit(size_t sz);
    template <typename T>
    T* allocator::worst_fit(size_t sz);
    template <typename T>
    T* allocator::best_fit(size_t sz);

    // auxilary
    template <typename T>
    T* allocator::req_blk(size_t sz);
    template <typename T>
    T* req_space(size_t sz);
};