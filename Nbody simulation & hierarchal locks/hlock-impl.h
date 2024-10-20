#include <mutex>
#include "hlock.h"

class HierarchicalMutex_impl : public HierarchicalMutex {
    std::mutex internal_mutex;
    int const hierarchy_value;
    int previous_hierarchy_value;
    static thread_local int this_thread_hierarchy_value;

    void check_for_hierarchy_violation() {
        if (this_thread_hierarchy_value >= hierarchy_value) {
            throw HierarchicalMutexException();
        }
    }

    void update_hierarchy_value() {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }

public:
    explicit HierarchicalMutex_impl(int lvl) : 
        HierarchicalMutex(lvl), 
        hierarchy_value(lvl), 
        previous_hierarchy_value(-1) 
    {}

    void lock() override {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }

    void unlock() override {
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }

    bool try_lock() override {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock()) {
            return false;
        }
        update_hierarchy_value();
        return true;
    }
};

thread_local int HierarchicalMutex_impl::this_thread_hierarchy_value = -1;
