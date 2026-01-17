#pragma once

#include <string>
#include <vector>

namespace cad {
namespace interop {

struct IoJob {
    std::string path;
    std::string format;
};

struct IoFormatSupport {
    std::string format;
    bool can_import{false};
    bool can_export{false};
};

struct IoJobResult {
    bool success{false};
    std::string message;
};

class IoPipeline {
public:
    IoJobResult importJob(const IoJob& job) const;
    IoJobResult exportJob(const IoJob& job) const;
    std::vector<IoFormatSupport> supportedFormats() const;
    bool supportsFormat(const std::string& format, bool is_export) const;
};

}  // namespace interop
}  // namespace cad
