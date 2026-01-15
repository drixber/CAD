#include "PerformanceMonitor.h"

namespace cad {
namespace core {

void PerformanceMonitor::recordFrame(double frame_ms) {
    frame_times_.push_back(frame_ms);
    while (frame_times_.size() > window_) {
        frame_times_.pop_front();
    }
}

double PerformanceMonitor::averageFrameMs() const {
    if (frame_times_.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (double value : frame_times_) {
        sum += value;
    }
    return sum / static_cast<double>(frame_times_.size());
}

double PerformanceMonitor::fps() const {
    const double avg = averageFrameMs();
    if (avg <= 0.0) {
        return 0.0;
    }
    return 1000.0 / avg;
}

void PerformanceMonitor::setWindowSize(std::size_t window) {
    window_ = window == 0 ? 1 : window;
    while (frame_times_.size() > window_) {
        frame_times_.pop_front();
    }
}

}  // namespace core
}  // namespace cad
