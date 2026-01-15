#pragma once

#include <cstddef>
#include <deque>

namespace cad {
namespace core {

class PerformanceMonitor {
public:
    void recordFrame(double frame_ms);
    double averageFrameMs() const;
    double fps() const;
    void setWindowSize(std::size_t window);

private:
    std::deque<double> frame_times_{};
    std::size_t window_{60};
};

}  // namespace core
}  // namespace cad
