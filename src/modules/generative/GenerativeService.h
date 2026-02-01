#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Generative Design (§18.10): Designraum, Lastfälle, Material, Fertigung, Varianten. Stub. */
class GenerativeService {
public:
    struct DesignSpace {
        std::string id;
        double min_x{0}, min_y{0}, min_z{0};
        double max_x{0}, max_y{0}, max_z{0};
    };
    std::string createStudy(const DesignSpace& space, const std::string& load_case_id);
    std::vector<std::string> getResultVariants(const std::string& study_id) const;
};

}  // namespace modules
}  // namespace cad
