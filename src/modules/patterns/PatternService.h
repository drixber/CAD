#pragma once

#include <string>

namespace cad {
namespace modules {

enum class PatternType {
    Rectangular,
    Circular,
    CurveDriven
};

struct PatternRequest {
    std::string targetFeature;
    PatternType type;
    int instanceCount{0};
};

struct PatternResult {
    bool success{false};
    std::string message;
};

class PatternService {
public:
    PatternResult createPattern(const PatternRequest& request) const;
};

}  // namespace modules
}  // namespace cad
