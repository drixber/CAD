#include "KernelFacade.h"

#include "core/logging/Logger.h"

namespace cad {
namespace kernel {

bool KernelFacade::initialize() {
    cad::core::Logger::instance().log(cad::core::Logger::Level::Info,
                                      "KernelFacade initialized");
    return true;
}

void KernelFacade::setModelRoot(const std::string& path) {
    model_root_ = path;
    cad::core::Logger::instance().log(cad::core::Logger::Level::Debug,
                                      "Kernel root set to " + model_root_);
}

const std::string& KernelFacade::modelRoot() const {
    return model_root_;
}

}  // namespace kernel
}  // namespace cad
