#pragma once

#include <string>

namespace cad {
namespace kernel {

class KernelFacade {
public:
    bool initialize();
    void setModelRoot(const std::string& path);
    const std::string& modelRoot() const;

private:
    std::string model_root_{};
};

}  // namespace kernel
}  // namespace cad
