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
    
    std::vector<RoutePoint> waypoints = getRouteWaypoints(route_id);
    waypoints.push_back(waypoint);
    
    RoutingRequest route_request;
    route_request.type = RoutingType::RigidPipe;
    if (!route.segments.empty()) {
        route_request.rigid_pipe_params.diameter = route.segments[0].diameter;
        route_request.rigid_pipe_params.material = route.segments[0].material;
    }
    
    route.segments = generateSegments(waypoints, RoutingType::RigidPipe, route_request);
    route.total_length = calculateRouteLength(route.segments);
    
    result = route;
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
    
    RoutingResult& route = const_cast<RoutingResult&>(it->second);
    
    std::vector<RoutePoint> waypoints = getRouteWaypoints(route_id);
    if (waypoint_index >= 0 && waypoint_index < static_cast<int>(waypoints.size())) {
        waypoints.erase(waypoints.begin() + waypoint_index);
        
        route.segments = generateSegments(waypoints, RoutingType::RigidPipe, 
                                          RoutingRequest());
        route.total_length = calculateRouteLength(route.segments);
        
        result = route;
        result.message = "Waypoint removed";
    } else {
        result.success = false;
        result.message = "Invalid waypoint index";
    }
    
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
    
    RoutingResult& route = const_cast<RoutingResult&>(it->second);
    
    std::vector<RoutePoint> waypoints = getRouteWaypoints(route_id);
    
    if (waypoints.size() < 3) {
        result = route;
        result.message = "Route optimized (too few waypoints)";
        return result;
    }
    
    std::vector<RoutePoint> optimized = optimizeWaypoints(waypoints, route.warnings);
    
    route.segments = generateSegments(optimized, RoutingType::RigidPipe, RoutingRequest());
    route.total_length = calculateRouteLength(route.segments);
    
    double original_length = calculateRouteLength(it->second.segments);
    double improvement = ((original_length - route.total_length) / original_length) * 100.0;
    
    result = route;
    result.message = "Route optimized: " + std::to_string(improvement) + "% shorter";
    
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
    
    std::vector<RoutePoint> path = findPath(start_connection, end_connection, obstacles);
    
    if (path.size() < 2) {
        request.waypoints = {RoutePoint(), RoutePoint()};
    } else {
        request.waypoints = path;
    }
    
    result = createRoute(request);
    result.message = "Auto-route completed: " + std::to_string(path.size()) + " waypoints";
    
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
    if (obstacle_ids.empty()) {
        return false;
    }
    
    for (const auto& segment : segments) {
        double segment_radius = segment.diameter * 0.5;
        
        for (const auto& obstacle_id : obstacle_ids) {
            std::hash<std::string> hasher;
            std::size_t hash = hasher(obstacle_id);
            
            double obstacle_x = static_cast<double>(hash % 1000) - 500.0;
            double obstacle_y = static_cast<double>((hash / 1000) % 1000) - 500.0;
            double obstacle_z = static_cast<double>((hash / 1000000) % 1000) - 500.0;
            double obstacle_radius = 50.0;
            
            double dx = (segment.start_point.x + segment.end_point.x) * 0.5 - obstacle_x;
            double dy = (segment.start_point.y + segment.end_point.y) * 0.5 - obstacle_y;
            double dz = (segment.start_point.z + segment.end_point.z) * 0.5 - obstacle_z;
            double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            if (dist < (segment_radius + obstacle_radius)) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<RoutePoint> RoutingService::optimizeWaypoints(const std::vector<RoutePoint>& waypoints,
                                                          const std::vector<std::string>& obstacles) const {
    if (waypoints.size() < 3) {
        return waypoints;
    }
    
    std::vector<RoutePoint> optimized = waypoints;
    bool changed = true;
    int max_iterations = 10;
    int iteration = 0;
    
    while (changed && iteration < max_iterations) {
        iteration++;
        changed = false;
        
        for (size_t i = 1; i < optimized.size() - 1; ++i) {
            RoutePoint& prev = optimized[i - 1];
            RoutePoint& curr = optimized[i];
            RoutePoint& next = optimized[i + 1];
            
            double dx1 = curr.x - prev.x;
            double dy1 = curr.y - prev.y;
            double dz1 = curr.z - prev.z;
            
            double dx2 = next.x - curr.x;
            double dy2 = next.y - curr.y;
            double dz2 = next.z - curr.z;
            
            double dot = dx1*dx2 + dy1*dy2 + dz1*dz2;
            double len1 = std::sqrt(dx1*dx1 + dy1*dy1 + dz1*dz1);
            double len2 = std::sqrt(dx2*dx2 + dy2*dy2 + dz2*dz2);
            
            if (len1 > 0.001 && len2 > 0.001) {
                double cos_angle = dot / (len1 * len2);
                
                if (cos_angle > 0.98) {
                    RoutePoint new_curr;
                    new_curr.x = (prev.x + next.x) * 0.5;
                    new_curr.y = (prev.y + next.y) * 0.5;
                    new_curr.z = (prev.z + next.z) * 0.5;
                    new_curr.radius = curr.radius;
                    new_curr.connection_id = curr.connection_id;
                    new_curr.is_fitting = curr.is_fitting;
                    
                    bool collision = false;
                    std::hash<std::string> hasher;
                    for (const auto& obstacle_id : obstacles) {
                        std::size_t obs_hash = hasher(obstacle_id);
                        double obs_x = static_cast<double>(obs_hash % 1000) - 500.0;
                        double obs_y = static_cast<double>((obs_hash / 1000) % 1000) - 500.0;
                        double obs_z = static_cast<double>((obs_hash / 1000000) % 1000) - 500.0;
                        double obs_r = 50.0;
                        
                        double dx = new_curr.x - obs_x;
                        double dy = new_curr.y - obs_y;
                        double dz = new_curr.z - obs_z;
                        double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                        
                        if (dist < obs_r + 10.0) {
                            collision = true;
                            break;
                        }
                    }
                    
                    if (!collision) {
                        curr = new_curr;
                        changed = true;
                    }
                }
            }
        }
    }
    
    std::vector<RoutePoint> final_optimized;
    final_optimized.push_back(optimized[0]);
    
    for (size_t i = 1; i < optimized.size() - 1; ++i) {
        double dx = optimized[i].x - final_optimized.back().x;
        double dy = optimized[i].y - final_optimized.back().y;
        double dz = optimized[i].z - final_optimized.back().z;
        double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        if (dist > 10.0) {
            final_optimized.push_back(optimized[i]);
        }
    }
    
    final_optimized.push_back(optimized.back());
    
    return final_optimized;
}

std::vector<RoutePoint> RoutingService::findPath(const std::string& start, const std::string& end,
                                                 const std::vector<std::string>& obstacles) const {
    std::vector<RoutePoint> path;
    
    std::hash<std::string> hasher;
    std::size_t start_hash = hasher(start);
    std::size_t end_hash = hasher(end);
    
    RoutePoint start_point;
    start_point.x = static_cast<double>(start_hash % 1000) - 500.0;
    start_point.y = static_cast<double>((start_hash / 1000) % 1000) - 500.0;
    start_point.z = static_cast<double>((start_hash / 1000000) % 1000) - 500.0;
    start_point.connection_id = start;
    
    RoutePoint end_point;
    end_point.x = static_cast<double>(end_hash % 1000) - 500.0;
    end_point.y = static_cast<double>((end_hash / 1000) % 1000) - 500.0;
    end_point.z = static_cast<double>((end_hash / 1000000) % 1000) - 500.0;
    end_point.connection_id = end;
    
    std::vector<RoutePoint> obstacle_points;
    std::vector<double> obstacle_radii;
    for (const auto& obstacle_id : obstacles) {
        std::size_t obs_hash = hasher(obstacle_id);
        RoutePoint obs_point;
        obs_point.x = static_cast<double>(obs_hash % 1000) - 500.0;
        obs_point.y = static_cast<double>((obs_hash / 1000) % 1000) - 500.0;
        obs_point.z = static_cast<double>((obs_hash / 1000000) % 1000) - 500.0;
        obstacle_points.push_back(obs_point);
        obstacle_radii.push_back(50.0 + static_cast<double>(obs_hash % 50));
    }
    
    struct PathNode {
        RoutePoint point;
        double g_cost{0.0};
        double h_cost{0.0};
        double f_cost{0.0};
        int parent{-1};
    };
    
    auto heuristic = [&](const RoutePoint& a, const RoutePoint& b) -> double {
        double dx = b.x - a.x;
        double dy = b.y - a.y;
        double dz = b.z - a.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    };
    
    auto checkObstacleCollision = [&](const RoutePoint& p, double radius = 10.0) -> bool {
        for (size_t i = 0; i < obstacle_points.size(); ++i) {
            double dx = p.x - obstacle_points[i].x;
            double dy = p.y - obstacle_points[i].y;
            double dz = p.z - obstacle_points[i].z;
            double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (dist < (radius + obstacle_radii[i])) {
                return true;
            }
        }
        return false;
    };
    
    std::vector<PathNode> open_set;
    std::vector<PathNode> closed_set;
    std::map<std::string, int> node_map;
    
    PathNode start_node;
    start_node.point = start_point;
    start_node.g_cost = 0.0;
    start_node.h_cost = heuristic(start_point, end_point);
    start_node.f_cost = start_node.g_cost + start_node.h_cost;
    start_node.parent = -1;
    open_set.push_back(start_node);
    node_map[start] = 0;
    
    int max_iterations = 1000;
    int iteration = 0;
    
    while (!open_set.empty() && iteration < max_iterations) {
        iteration++;
        
        int best_idx = 0;
        double best_f = open_set[0].f_cost;
        for (size_t i = 1; i < open_set.size(); ++i) {
            if (open_set[i].f_cost < best_f) {
                best_f = open_set[i].f_cost;
                best_idx = static_cast<int>(i);
            }
        }
        
        PathNode current = open_set[best_idx];
        open_set.erase(open_set.begin() + best_idx);
        closed_set.push_back(current);
        
        double dist_to_end = heuristic(current.point, end_point);
        if (dist_to_end < 20.0) {
            std::vector<RoutePoint> final_path;
            int node_idx = static_cast<int>(closed_set.size()) - 1;
            while (node_idx >= 0) {
                final_path.insert(final_path.begin(), closed_set[node_idx].point);
                node_idx = closed_set[node_idx].parent;
            }
            final_path.push_back(end_point);
            return final_path;
        }
        
        std::vector<RoutePoint> neighbors;
        double step_size = 50.0;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dz = -1; dz <= 1; ++dz) {
                    if (dx == 0 && dy == 0 && dz == 0) continue;
                    RoutePoint neighbor;
                    neighbor.x = current.point.x + dx * step_size;
                    neighbor.y = current.point.y + dy * step_size;
                    neighbor.z = current.point.z + dz * step_size;
                    if (!checkObstacleCollision(neighbor)) {
                        neighbors.push_back(neighbor);
                    }
                }
            }
        }
        
        for (const auto& neighbor : neighbors) {
            bool in_closed = false;
            for (const auto& closed : closed_set) {
                double dist = heuristic(neighbor, closed.point);
                if (dist < 1.0) {
                    in_closed = true;
                    break;
                }
            }
            if (in_closed) continue;
            
            double g_new = current.g_cost + heuristic(current.point, neighbor);
            double h_new = heuristic(neighbor, end_point);
            double f_new = g_new + h_new;
            
            bool in_open = false;
            for (auto& open : open_set) {
                double dist = heuristic(neighbor, open.point);
                if (dist < 1.0) {
                    if (f_new < open.f_cost) {
                        open.g_cost = g_new;
                        open.h_cost = h_new;
                        open.f_cost = f_new;
                        open.parent = static_cast<int>(closed_set.size()) - 1;
                    }
                    in_open = true;
                    break;
                }
            }
            
            if (!in_open) {
                PathNode new_node;
                new_node.point = neighbor;
                new_node.g_cost = g_new;
                new_node.h_cost = h_new;
                new_node.f_cost = f_new;
                new_node.parent = static_cast<int>(closed_set.size()) - 1;
                open_set.push_back(new_node);
            }
        }
    }
    
    path.push_back(start_point);
    path.push_back(end_point);
    return path;
}

}  // namespace modules
}  // namespace cad
