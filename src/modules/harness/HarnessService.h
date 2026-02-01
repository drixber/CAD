#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

/** Ein Kabel/Leitung im Harness. */
struct HarnessCable {
    std::string cable_id;
    std::string name;
    std::string part_number;
    double cross_section_mm2{0.0};
    std::string color;
    std::vector<std::string> waypoint_ids;  /** Reihenfolge der Punkte für Routen */
};

/** Ein Routen-Punkt (im Raum). */
struct HarnessWaypoint {
    std::string waypoint_id;
    double x{0.0}, y{0.0}, z{0.0};
    std::string connection_id;  /** Optional: Stecker/Kontakt */
};

/** Ein Bündel: mehrere Kabel gemeinsam geführt. */
struct HarnessBundle {
    std::string bundle_id;
    std::string name;
    std::vector<std::string> cable_ids;
    std::vector<std::string> waypoint_ids;  /** Gemeinsamer Pfad */
};

/** Harness (Kabelbaum) pro Baugruppe. */
struct Harness {
    std::string harness_id;
    std::string assembly_id;
    std::vector<HarnessCable> cables;
    std::vector<HarnessWaypoint> waypoints;
    std::vector<HarnessBundle> bundles;
};

class HarnessService {
public:
    /** Kabelbaum für Baugruppe anlegen/abrufen. */
    std::string createHarness(const std::string& assembly_id);
    Harness getHarness(const std::string& harness_id) const;

    /** Kabel routen: Wegpunkte setzen, Kabellänge berechnen. */
    bool addCable(const std::string& harness_id, const HarnessCable& cable);
    bool setCableRoute(const std::string& harness_id, const std::string& cable_id,
                       const std::vector<std::string>& waypoint_ids);
    std::vector<HarnessCable> getCables(const std::string& harness_id) const;
    /** Kabellänge für ein Kabel (Summe der Segmentlängen). */
    double getCableLength(const std::string& harness_id, const std::string& cable_id) const;

    /** Wegpunkte. */
    bool addWaypoint(const std::string& harness_id, const HarnessWaypoint& wp);
    std::vector<HarnessWaypoint> getWaypoints(const std::string& harness_id) const;

    /** Bündel: mehrere Kabel gemeinsam führen. */
    bool addBundle(const std::string& harness_id, const HarnessBundle& bundle);
    std::vector<HarnessBundle> getBundles(const std::string& harness_id) const;

private:
    mutable std::map<std::string, Harness> harnesses_;
    double segmentLength(double x1, double y1, double z1, double x2, double y2, double z2) const;
    HarnessWaypoint getWaypointById(const Harness& h, const std::string& id) const;
};

}  // namespace modules
}  // namespace cad
