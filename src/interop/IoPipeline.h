#pragma once

#include <string>

namespace cad {
namespace interop {

struct IoJob {
    std::string path;
    std::string format;
};

struct IoJobResult {
    bool success{false};
    std::string message;
};

class IoPipeline {
public:
    IoJobResult importJob(const IoJob& job) const;
    IoJobResult exportJob(const IoJob& job) const;
};

}  // namespace interop
}  // namespace cad
