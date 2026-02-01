#pragma once

#include <string>
#include <vector>

namespace cad {
namespace modules {

/** Electronics (§18.15): Schaltplan, PCB, Routing, 3D-PCB in Baugruppe. Stub. */
class ElectronicsService {
public:
    std::string createSchematic(const std::string& name);
    std::string createPcb(const std::string& schematic_id);
    bool routePcb(const std::string& pcb_id);
    std::string exportTo3D(const std::string& pcb_id);
};

}  // namespace modules
}  // namespace cad
