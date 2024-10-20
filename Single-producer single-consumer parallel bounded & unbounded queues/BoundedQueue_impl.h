#include "BoundedQueue.h"
#include <mutex>
#include <condition_variable>
#include <vector>

class BoundedQueue : public BoundedQueueAbstract {
private:
    std::vector<int> buffer;
    int capacity;
    int head;
    int tail;
    int count;
    std::mutex mtx;
    std::condition_variable not_full;
    std::condition_variable not_empty;

public:
    BoundedQueue(int n) : capacity(n), head(0), tail(0), count(0) {
        buffer.resize(capacity);
    }

    int size() override {
        std::lock_guard<std::mutex> lock(mtx);
        return count;
    }

    int pop() override {
        std::unique_lock<std::mutex> lock(mtx);
        while (count == 0) {
            not_empty.wait(lock);
        }
        int value = buffer[head];
        head = (head + 1) % capacity;
        count--;
        not_full.notify_one();
        return value;
    }

    void push(int v) override {
        std::unique_lock<std::mutex> lock(mtx);
        while (count == capacity) {
            not_full.wait(lock);
        }
        buffer[tail] = v;
        tail = (tail + 1) % capacity;
        count++;
        not_empty.notify_one();
    }

};
