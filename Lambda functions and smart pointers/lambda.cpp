#include "lambda.h"
#include <cmath>
#include <algorithm>
#include <iostream>

void init_globals() {
    by_value = [](std::vector<int> vec) {
        std::vector<int> evens;
        for (int num : vec) {
            if (num % 2 == 0) {
                evens.push_back(num);
            }
        }
        return evens;
    };

    by_ref = [](std::vector<int>& vec) {
        std::vector<int> evens;
        for (int num : vec) {
            if (num % 2 == 0) {
                evens.push_back(num);
            }
        }
        return evens;
    };

    cmp_lambda = [](int &a, int &b) {
        g_counter++;
        return std::abs(a) < std::abs(b);
    };
}

std::vector<int> ascending_sort(std::vector<int>& vec) {
    init_globals();
    
    std::sort(vec.begin(), vec.end(), cmp_lambda);

    std::cout << g_counter << std::endl;

    return vec;
}
