#include "UnboundedQueue1p1c.h"
#include <atomic>
#include <cstdlib>

class UnboundedQueue1p1c : public UnboundedQueue1p1cAbstract {
private:
    struct Node {
        int value;
        Node* next;
        Node(int val) : value(val), next(nullptr) {}
    };

    Node* dummy;
    std::atomic<Node*> head;
    std::atomic<Node*> tail;

public:
    UnboundedQueue1p1c() {
        dummy = new Node(0);  
        head = tail = dummy;
    }

    bool pop(int &val) override {
        Node* old_head = head.load(std::memory_order_acquire);
        Node* next = old_head->next;
        if (next == nullptr) {
            return false;  
        }
        val = next->value;
        head.store(next, std::memory_order_release);
        delete old_head; 
        return true;
    }

    void push(int value) override {
        Node* new_node = new Node(value);
        Node* old_tail = tail.load(std::memory_order_acquire);
        old_tail->next = new_node;
        tail.store(new_node, std::memory_order_release);
    }

    int size() const override {
        int count = 0;
        Node* current = head.load(std::memory_order_acquire)->next;
        while (current != nullptr) {
            count++;
            current = current->next;
        }
        return count;
    }

    ~UnboundedQueue1p1c() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }
};
