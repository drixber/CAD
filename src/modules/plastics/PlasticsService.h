#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Plastics / Spritzguss (SolidWorks Plastics, §19.9): Füllanalyse, Schweißlinien, Einfallstellen. Stub. */
class PlasticsService {
public:
    struct FillResult {
        bool success{false};
        double fill_time_ms{0};
        std::vector<double> weld_line_positions;
    };
    FillResult runFillAnalysis(const std::string& part_id) const;
    bool runWeldLineAnalysis(const std::string& part_id) const;
    bool runSinkMarkAnalysis(const std::string& part_id) const;
};

}  // namespace modules
}  // namespace cad
