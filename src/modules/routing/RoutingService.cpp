#include "RoutingService.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace cad {
namespace modules {

RoutingResult RoutingService::createRoute(const RoutingRequest& request) const {
    RoutingResult result;
    
    if (request.targetAssembly.empty()) {
        result.success = false;
        result.message = "No target assembly specified";
        return result;
    }
    
    if (request.waypoints.size() < 2) {
        result.success = false;
        result.message = "At least 2 waypoints required";
        return result;
    }
    
    switch (request.type) {
        case RoutingType::RigidPipe:
            return createRigidPipe(request);
        case RoutingType::FlexibleHose:
            return createFlexibleHose(request);
        case RoutingType::BentTube:
            return createBentTube(request);
        default:
            result.success = false;
            result.message = "Unknown routing type";
            return result;
    }
}

RoutingResult RoutingService::createRigidPipe(const RoutingRequest& request) const {
    RoutingResult result;
    result.success = true;
    result.message = "Rigid pipe route created";
    result.route_id = request.targetAssembly + "_rigid_pipe";
    
    // Generate segments
    result.segments = generateSegments(request.waypoints, RoutingType::RigidPipe, request);
    result.total_length = calculateRouteLength(result.segments);
    result.fitting_count = request.rigid_pipe_params.fitting_ids.size();
    
    // Check for collisions
    if (!request.obstacle_ids.empty()) {
        if (checkCollisions(result.segments, request.obstacle_ids)) {
            result.warnings.push_back("Route intersects with obstacles");
        }
    }
    
    routes_[result.route_id] = result;
    
    return result;
}

RoutingResult RoutingService::createFlexibleHose(const RoutingRequest& request) const {
    RoutingResult result;
    result.success = true;
    result.message = "Flexible hose route created";
    result.route_id = request.targetAssembly + "_flexible_hose";
    
    // Generate segments
    result.segments = generateSegments(request.waypoints, RoutingType::FlexibleHose, request);
    result.total_length = calculateRouteLength(result.segments);
    
    // Check max length
    if (result.total_length > request.flexible_hose_params.max_length) {
        result.warnings.push_back("Route length exceeds maximum allowed length");
    }
    
    routes_[result.route_id] = result;
    
    return result;
}

RoutingResult RoutingService::createBentTube(const RoutingRequest& request) const {
    RoutingResult result;
    result.success = true;
    result.message = "Bent tube route created";
    result.route_id = request.targetAssembly + "_bent_tube";
    
    // Generate segments
    result.segments = generateSegments(request.waypoints, RoutingType::BentTube, request);
    result.total_length = calculateRouteLength(result.segments);
    
    // Count bends
    int bend_count = static_cast<int>(result.segments.size()) - 1;
    if (bend_count > request.bent_tube_params.bend_count && request.bent_tube_params.bend_count > 0) {
        result.warnings.push_back("Bend count exceeds specified limit");
    }
    
    routes_[result.route_id] = result;
    
    return result;
}

RoutingResult RoutingService::editRoute(const std::string& route_id, const RoutingRequest& request) const {
    RoutingResult result;
    
    auto it = routes_.find(route_id);
    if (it == routes_.end()) {
        result.success = false;
        result.message = "Route not found";
        return result;
    }
    
    // Recreate route with new parameters
    return createRoute(request);
}

RoutingResult RoutingService::addWaypoint(const std::string& route_id, const RoutePoint& waypoint) const {
    RoutingResult result;
    
    auto it = routes_.find(route_id);
    if (it == routes_.end()) {
        result.success = false;
        result.message = "Route not found";
        return result;
    }
    
    RoutingResult& route = const_cast<RoutingResult&>(it->second);
    
    // Add waypoint and regenerate segments
    // In real implementation: would update route waypoints and regenerate
    result.success = true;
    result.message = "Waypoint added";
    
    return result;
}

RoutingResult RoutingService::removeWaypoint(const std::string& route_id, int waypoint_index) const {
    RoutingResult result;
    
    auto it = routes_.find(route_id);
    if (it == routes_.end()) {
        result.success = false;
        result.message = "Route not found";
        return result;
    }
    
    // Remove waypoint and regenerate segments
    // In real implementation: would update route waypoints and regenerate
    result.success = true;
    result.message = "Waypoint removed";
    
    return result;
}

RoutingResult RoutingService::optimizeRoute(const std::string& route_id) const {
    RoutingResult result;
    
    auto it = routes_.find(route_id);
    if (it == routes_.end()) {
        result.success = false;
        result.message = "Route not found";
        return result;
    }
    
    // Optimize route waypoints
    // In real implementation: would use pathfinding algorithm
    result.success = true;
    result.message = "Route optimized";
    
    return result;
}

RoutingResult RoutingService::autoRoute(const std::string& start_connection, const std::string& end_connection,
                                       RoutingType type, const std::vector<std::string>& obstacles) const {
    RoutingResult result;
    
    // Auto-route between connections
    RoutePoint start;
    start.connection_id = start_connection;
    
    RoutePoint end;
    end.connection_id = end_connection;
    
    RoutingRequest request;
    request.type = type;
    request.waypoints = {start, end};
    request.auto_route = true;
    request.obstacle_ids = obstacles;
    
    // In real implementation: would use pathfinding algorithm
    result = createRoute(request);
    result.message = "Auto-route completed";
    
    return result;
}

std::vector<RouteSegment> RoutingService::getRouteSegments(const std::string& route_id) const {
    auto it = routes_.find(route_id);
    if (it != routes_.end()) {
        return it->second.segments;
    }
    return {};
}

double RoutingService::getRouteLength(const std::string& route_id) const {
    auto it = routes_.find(route_id);
    if (it != routes_.end()) {
        return it->second.total_length;
    }
    return 0.0;
}

std::vector<RoutePoint> RoutingService::getRouteWaypoints(const std::string& route_id) const {
    auto it = routes_.find(route_id);
    if (it != routes_.end() && !it->second.segments.empty()) {
        std::vector<RoutePoint> waypoints;
        for (const auto& segment : it->second.segments) {
            waypoints.push_back(segment.start_point);
        }
        if (!it->second.segments.empty()) {
            waypoints.push_back(it->second.segments.back().end_point);
        }
        return waypoints;
    }
    return {};
}

std::vector<RouteSegment> RoutingService::generateSegments(const std::vector<RoutePoint>& waypoints,
                                                           RoutingType type, const RoutingRequest& request) const {
    std::vector<RouteSegment> segments;
    
    for (size_t i = 0; i < waypoints.size() - 1; ++i) {
        RouteSegment segment;
        segment.segment_id = "segment_" + std::to_string(i);
        segment.start_point = waypoints[i];
        segment.end_point = waypoints[i + 1];
        
        // Calculate length
        double dx = segment.end_point.x - segment.start_point.x;
        double dy = segment.end_point.y - segment.start_point.y;
        double dz = segment.end_point.z - segment.start_point.z;
        segment.length = std::sqrt(dx * dx + dy * dy + dz * dz);
        
        // Set diameter based on type
        switch (type) {
            case RoutingType::RigidPipe:
                segment.diameter = request.rigid_pipe_params.diameter;
                segment.material = request.rigid_pipe_params.material;
                break;
            case RoutingType::FlexibleHose:
                segment.diameter = request.flexible_hose_params.diameter;
                segment.material = request.flexible_hose_params.material;
                break;
            case RoutingType::BentTube:
                segment.diameter = request.bent_tube_params.diameter;
                segment.material = request.bent_tube_params.material;
                break;
        }
        
        segments.push_back(segment);
    }
    
    return segments;
}

double RoutingService::calculateRouteLength(const std::vector<RouteSegment>& segments) const {
    double total_length = 0.0;
    for (const auto& segment : segments) {
        total_length += segment.length;
    }
    return total_length;
}

bool RoutingService::checkCollisions(const std::vector<RouteSegment>& segments,
                                     const std::vector<std::string>& obstacle_ids) const {
    // Simplified collision check
    // In real implementation: would check against actual obstacle geometry
    return false;  // Assume no collisions for now
}

std::vector<RoutePoint> RoutingService::optimizeWaypoints(const std::vector<RoutePoint>& waypoints,
                                                          const std::vector<std::string>& obstacles) const {
    // Simplified optimization: return waypoints as-is
    // In real implementation: would use pathfinding algorithm
    return waypoints;
}

}  // namespace modules
}  // namespace cad
