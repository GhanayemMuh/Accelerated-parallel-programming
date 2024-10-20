#ifndef CDF_H
#define CDF_H

#include "functor.h"
#include <vector>

class Accumulator : public IAccumulator {
private:
    float total = 0.0;

public:
    float operator() (const float &a) override {
        total += a;
        return total;
    }
};

template<typename F>
std::vector<float> cdf(const std::vector<float>& pdfv, F& acc) {
    std::vector<float> result;
    result.reserve(pdfv.size());

    for (const auto& value : pdfv) {
        result.push_back(acc(value));
    }

    return result;
}

#endif // CDF_H
