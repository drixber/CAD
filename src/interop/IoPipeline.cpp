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

}  // namespace interop
}  // namespace cad
