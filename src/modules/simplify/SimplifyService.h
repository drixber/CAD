#pragma once

#include <string>

namespace cad {
namespace modules {

struct SimplifyRequest {
    std::string targetAssembly;
    std::string replacementType;
};

struct SimplifyResult {
    bool success{false};
    std::string message;
};

class SimplifyService {
public:
    SimplifyResult simplify(const SimplifyRequest& request) const;
};

}  // namespace modules
}  // namespace cad
