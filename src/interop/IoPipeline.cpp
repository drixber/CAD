#include "IoPipeline.h"

namespace cad {
namespace interop {

IoJobResult IoPipeline::importJob(const IoJob& job) const {
    IoJobResult result;
    if (!job.path.empty()) {
        result.success = true;
        result.message = "IO import queued";
    }
    return result;
}

IoJobResult IoPipeline::exportJob(const IoJob& job) const {
    IoJobResult result;
    if (!job.path.empty()) {
        result.success = true;
        result.message = "IO export queued";
    }
    return result;
}

std::vector<IoFormatSupport> IoPipeline::supportedFormats() const {
    return {
        {"STEP", true, true},
        {"IGES", true, true},
        {"STL", true, true},
        {"DWG", true, true},
        {"DXF", true, true},
        {"SAT", true, true},
        {"RFA", false, true},
    };
}

bool IoPipeline::supportsFormat(const std::string& format, bool is_export) const {
    for (const auto& supported : supportedFormats()) {
        if (supported.format == format) {
            return is_export ? supported.can_export : supported.can_import;
        }
    }
    return false;
}

}  // namespace interop
}  // namespace cad
