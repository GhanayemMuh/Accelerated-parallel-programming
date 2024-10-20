#include "BoundedQueue1p1c.h"
#include <atomic>

class BoundedQueue1p1c : public BoundedQueueAbstract_1p1c {
private:
    int* queue;
    std::atomic<int> head;
    std::atomic<int> tail;
    std::atomic<int> count;
    int capacity;

public:
    BoundedQueue1p1c(int cap) : capacity(cap), head(0), tail(0), count(0) {
        queue = new int[capacity];
    }

    ~BoundedQueue1p1c() {
        delete[] queue;
    }

    virtual int size() override {
        return count.load();
    }

    virtual bool pop(int &val) override {
        int currentCount = count.load();
        if (currentCount == 0) {
            return false;
        }
        int currentHead = head.load();
        val = queue[currentHead];
        head.store((currentHead + 1) % capacity);
        count.fetch_sub(1);
        return true;
    }

    virtual bool push(int v) override {
        int currentCount = count.load();
        if (currentCount == capacity) {
            return false;
        }
        int currentTail = tail.load();
        queue[currentTail] = v;
        tail.store((currentTail + 1) % capacity);
        count.fetch_add(1);
        return true;
    }
};
