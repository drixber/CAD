#include <gtest/gtest.h>
#include "modules/routing/RoutingService.h"

using namespace cad::modules;

TEST(RoutingServiceTest, CreateRigidPipe) {
    RoutingService service;
    
    RoutingRequest request;
    request.targetAssembly = "test_assembly";
    request.type = RoutingType::RigidPipe;
    
    RoutePoint start;
    start.x = 0.0;
    start.y = 0.0;
    start.z = 0.0;
    
    RoutePoint end;
    end.x = 100.0;
    end.y = 100.0;
    end.z = 50.0;
    
    request.waypoints = {start, end};
    request.rigid_pipe_params.diameter = 10.0;
    
    RoutingResult result = service.createRigidPipe(request);
    
    ASSERT_TRUE(result.success);
    ASSERT_FALSE(result.segments.empty());
    ASSERT_GT(result.total_length, 0.0);
}

TEST(RoutingServiceTest, Pathfinding) {
    RoutingService service;
    
    std::vector<std::string> obstacles = {"obstacle1", "obstacle2"};
    
    std::vector<RoutePoint> path = service.findPath("start", "end", obstacles);
    
    ASSERT_GE(path.size(), 2);
}

TEST(RoutingServiceTest, OptimizeWaypoints) {
    RoutingService service;
    
    std::vector<RoutePoint> waypoints;
    for (int i = 0; i < 5; ++i) {
        RoutePoint p;
        p.x = static_cast<double>(i * 10);
        p.y = static_cast<double>(i * 10);
        p.z = 0.0;
        waypoints.push_back(p);
    }
    
    std::vector<std::string> obstacles;
    std::vector<RoutePoint> optimized = service.optimizeWaypoints(waypoints, obstacles);
    
    ASSERT_LE(optimized.size(), waypoints.size());
}

TEST(RoutingServiceTest, AutoRoute) {
    RoutingService service;
    
    RoutingResult result = service.autoRoute("start_conn", "end_conn", 
                                             RoutingType::FlexibleHose, {});
    
    ASSERT_TRUE(result.success);
}
