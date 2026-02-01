#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Composite Design (§22.7): Faserverbund, Lagenaufbau, CFK, Luftfahrtstrukturen. Stub. */
class CompositesService {
public:
    struct Ply {
        std::string id;
        std::string material;
        double thickness_mm{0};
        double angle_deg{0};
        int sequence{0};
    };
    std::string createCompositePart(const std::string& name);
    bool addPly(const std::string& part_id, const Ply& ply);
    std::vector<Ply> getPlies(const std::string& part_id) const;
    bool exportLayupReport(const std::string& part_id, const std::string& path) const;
};

}  // namespace modules
}  // namespace cad
