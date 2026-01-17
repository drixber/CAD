#pragma once

#include <string>
#include <chrono>

namespace cad {
namespace core {

struct PerfSpan {
    std::string name;
    double elapsed_ms{0.0};
};

class PerfTimer {
public:
    explicit PerfTimer(std::string label);
    PerfSpan finish() const;

private:
    std::string label_;
    std::chrono::steady_clock::time_point start_;
};

}  // namespace core
}  // namespace cad
