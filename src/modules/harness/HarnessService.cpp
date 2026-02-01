#include "HarnessService.h"
#include <cmath>

namespace cad {
namespace modules {

double HarnessService::segmentLength(double x1, double y1, double z1, double x2, double y2, double z2) const {
    double dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

std::string HarnessService::createHarness(const std::string& assembly_id) {
    std::string id = "H_" + assembly_id + "_" + std::to_string(harnesses_.size() + 1);
    Harness h;
    h.harness_id = id;
    h.assembly_id = assembly_id;
    harnesses_[id] = h;
    return id;
}

Harness HarnessService::getHarness(const std::string& harness_id) const {
    auto it = harnesses_.find(harness_id);
    if (it != harnesses_.end()) return it->second;
    return {};
}

HarnessWaypoint HarnessService::getWaypointById(const Harness& h, const std::string& id) const {
    for (const auto& w : h.waypoints) {
        if (w.waypoint_id == id) return w;
    }
    return {};
}

bool HarnessService::addWaypoint(const std::string& harness_id, const HarnessWaypoint& wp) {
    auto it = harnesses_.find(harness_id);
    if (it == harnesses_.end()) return false;
    HarnessWaypoint w = wp;
    if (w.waypoint_id.empty()) {
        w.waypoint_id = "WP" + std::to_string(it->second.waypoints.size() + 1);
    }
    it->second.waypoints.push_back(w);
    return true;
}

bool HarnessService::addCable(const std::string& harness_id, const HarnessCable& cable) {
    auto it = harnesses_.find(harness_id);
    if (it == harnesses_.end()) return false;
    HarnessCable c = cable;
    if (c.cable_id.empty()) {
        c.cable_id = "C" + std::to_string(it->second.cables.size() + 1);
    }
    it->second.cables.push_back(c);
    return true;
}

bool HarnessService::setCableRoute(const std::string& harness_id, const std::string& cable_id,
                                    const std::vector<std::string>& waypoint_ids) {
    auto it = harnesses_.find(harness_id);
    if (it == harnesses_.end()) return false;
    for (auto& c : it->second.cables) {
        if (c.cable_id == cable_id) {
            c.waypoint_ids = waypoint_ids;
            return true;
        }
    }
    return false;
}

std::vector<HarnessCable> HarnessService::getCables(const std::string& harness_id) const {
    auto it = harnesses_.find(harness_id);
    if (it != harnesses_.end()) return it->second.cables;
    return {};
}

double HarnessService::getCableLength(const std::string& harness_id, const std::string& cable_id) const {
    Harness h = getHarness(harness_id);
    const HarnessCable* cable = nullptr;
    for (const auto& c : h.cables) {
        if (c.cable_id == cable_id) { cable = &c; break; }
    }
    if (!cable || cable->waypoint_ids.size() < 2) return 0.0;
    double total = 0.0;
    HarnessWaypoint prev = getWaypointById(h, cable->waypoint_ids[0]);
    for (size_t i = 1; i < cable->waypoint_ids.size(); ++i) {
        HarnessWaypoint curr = getWaypointById(h, cable->waypoint_ids[i]);
        total += segmentLength(prev.x, prev.y, prev.z, curr.x, curr.y, curr.z);
        prev = curr;
    }
    return total;
}

std::vector<HarnessWaypoint> HarnessService::getWaypoints(const std::string& harness_id) const {
    auto it = harnesses_.find(harness_id);
    if (it != harnesses_.end()) return it->second.waypoints;
    return {};
}

bool HarnessService::addBundle(const std::string& harness_id, const HarnessBundle& bundle) {
    auto it = harnesses_.find(harness_id);
    if (it == harnesses_.end()) return false;
    HarnessBundle b = bundle;
    if (b.bundle_id.empty()) {
        b.bundle_id = "B" + std::to_string(it->second.bundles.size() + 1);
    }
    it->second.bundles.push_back(b);
    return true;
}

std::vector<HarnessBundle> HarnessService::getBundles(const std::string& harness_id) const {
    auto it = harnesses_.find(harness_id);
    if (it != harnesses_.end()) return it->second.bundles;
    return {};
}

}  // namespace modules
}  // namespace cad
