#include <iostream>
#include <chrono>
#include <memory>

void useRawPointers() {
    int iterations = 1000000;
    for (int i = 0; i < iterations; ++i) {
        int* ptr = new int(i);
        *ptr *= 2;
        delete ptr;
    }
}

void useSmartPointers() {
    int iterations = 1000000;
    for (int i = 0; i < iterations; ++i) {
        std::unique_ptr<int> ptr(new int(i));
        *ptr *= 2;
    }
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    useRawPointers();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Raw Pointers: " << elapsed.count() << " seconds.\n";

    start = std::chrono::high_resolution_clock::now();
    useSmartPointers();
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    std::cout << "Smart Pointers: " << elapsed.count() << " seconds.\n";
    
    return 0;
}
