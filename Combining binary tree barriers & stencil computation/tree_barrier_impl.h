#include "tree_barrier.h"
#include <atomic>
#include <memory>
#include <thread>
#include <iostream>
#include <vector>

class BinaryTreeBarrier : public BinaryTreeBarrierAbstract {
private:
    std::unique_ptr<std::atomic<int>[]> node_arrivals; 
    int num_threads;

public:
    BinaryTreeBarrier(int n) : num_threads(n) {
        if (num_threads > 1) {
            
            node_arrivals = std::unique_ptr<std::atomic<int>[]>(new std::atomic<int>[(num_threads - 1) / 2]);
            for (int i = 0; i < (num_threads - 1) / 2; ++i) {
                node_arrivals[i].store(0, std::memory_order_relaxed);
            }
        }
    }

    void barrier() override {

        int local_thread_id = thread_id;
        int index = local_thread_id / 2; 

       
        while (index < (num_threads - 1) / 2) { 
            int parent = (index - 1) / 2;
            int arrival_count = node_arrivals[index].fetch_add(1, std::memory_order_acq_rel) + 1;
            if (arrival_count < 2) {
                while (node_arrivals[index].load(std::memory_order_acquire) < 2) {
                    std::this_thread::yield(); 
                }
            }
            if (index == 0) break; 
            index = parent; 
        }

        if (local_thread_id == 0) { 
            for (int i = 0; i < (num_threads - 1) / 2; ++i) {
                node_arrivals[i].store(0, std::memory_order_relaxed); 
            }
        }
    }
};