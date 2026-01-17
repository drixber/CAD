#include "PerfSpan.h"

namespace cad {
namespace core {

PerfTimer::PerfTimer(std::string label)
    : label_(std::move(label)), start_(std::chrono::steady_clock::now()) {}

PerfSpan PerfTimer::finish() const {
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start_;
    PerfSpan span;
    span.name = label_;
    span.elapsed_ms = elapsed.count();
    return span;
}

}  // namespace core
}  // namespace cad
