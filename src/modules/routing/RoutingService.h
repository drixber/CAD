#pragma once

#include <string>
#include <vector>
#include <map>

namespace cad {
namespace modules {

enum class RoutingType {
    RigidPipe,
    FlexibleHose,
    BentTube
};

struct RoutePoint {
    double x{0.0};
    double y{0.0};
    double z{0.0};
    double radius{0.0};
    std::string connection_id;
    bool is_fitting{false};
};

struct RouteSegment {
    std::string segment_id;
    RoutePoint start_point;
    RoutePoint end_point;
    double length{0.0};
    double diameter{10.0};
    std::string material;
};

struct RigidPipeParams {
    double diameter{10.0};
    double wall_thickness{1.0};
    std::string material{"Steel"};
    double min_bend_radius{50.0};
    std::vector<std::string> fitting_ids;
};

struct FlexibleHoseParams {
    double diameter{10.0};
    double min_bend_radius{30.0};
    double max_length{1000.0};
    std::string material{"Rubber"};
};

struct BentTubeParams {
    double diameter{10.0};
    double wall_thickness{1.0};
    double bend_radius{50.0};
    int bend_count{0};
    std::string material{"Steel"};
};

struct RoutingRequest {
    std::string targetAssembly;
    RoutingType type;
    std::vector<RoutePoint> waypoints;
    RigidPipeParams rigid_pipe_params;
    FlexibleHoseParams flexible_hose_params;
    BentTubeParams bent_tube_params;
    bool auto_route{false};
    std::vector<std::string> obstacle_ids;
};

struct RoutingResult {
    bool success{false};
    std::string message;
    std::string route_id;
    std::vector<RouteSegment> segments;
    double total_length{0.0};
    int fitting_count{0};
    std::vector<std::string> warnings;
};

class RoutingService {
public:
    RoutingResult createRoute(const RoutingRequest& request) const;
    RoutingResult createRigidPipe(const RoutingRequest& request) const;
    RoutingResult createFlexibleHose(const RoutingRequest& request) const;
    RoutingResult createBentTube(const RoutingRequest& request) const;
    
    // Route editing
    RoutingResult editRoute(const std::string& route_id, const RoutingRequest& request) const;
    RoutingResult addWaypoint(const std::string& route_id, const RoutePoint& waypoint) const;
    RoutingResult removeWaypoint(const std::string& route_id, int waypoint_index) const;
    RoutingResult optimizeRoute(const std::string& route_id) const;
    
    // Route queries
    std::vector<RouteSegment> getRouteSegments(const std::string& route_id) const;
    double getRouteLength(const std::string& route_id) const;
    std::vector<RoutePoint> getRouteWaypoints(const std::string& route_id) const;
    
    // Auto-routing
    RoutingResult autoRoute(const std::string& start_connection, const std::string& end_connection, 
                           RoutingType type, const std::vector<std::string>& obstacles) const;
    
private:
    mutable std::map<std::string, RoutingResult> routes_;
    
    std::vector<RouteSegment> generateSegments(const std::vector<RoutePoint>& waypoints, 
                                               RoutingType type, const RoutingRequest& request) const;
    double calculateRouteLength(const std::vector<RouteSegment>& segments) const;
    bool checkCollisions(const std::vector<RouteSegment>& segments, 
                        const std::vector<std::string>& obstacle_ids) const;
    std::vector<RoutePoint> optimizeWaypoints(const std::vector<RoutePoint>& waypoints,
                                              const std::vector<std::string>& obstacles) const;
    std::vector<RoutePoint> findPath(const std::string& start, const std::string& end,
                                     const std::vector<std::string>& obstacles) const;
};

}  // namespace modules
}  // namespace cad
