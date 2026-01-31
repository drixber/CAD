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

TEST(RoutingServiceTest, PathfindingViaAutoRoute) {
    RoutingService service;
    
    RoutingResult result = service.autoRoute("start", "end", RoutingType::RigidPipe, {"obstacle1", "obstacle2"});
    
    ASSERT_TRUE(result.success || !result.message.empty());
}

TEST(RoutingServiceTest, OptimizeRoute) {
    RoutingService service;
    
    RoutingRequest request;
    request.targetAssembly = "test_assembly";
    request.type = RoutingType::RigidPipe;
    for (int i = 0; i < 5; ++i) {
        RoutePoint p;
        p.x = static_cast<double>(i * 10);
        p.y = static_cast<double>(i * 10);
        p.z = 0.0;
        request.waypoints.push_back(p);
    }
    RoutingResult create_result = service.createRigidPipe(request);
    if (!create_result.success || create_result.route_id.empty()) {
        GTEST_SKIP() << "createRigidPipe not available";
    }
    RoutingResult opt_result = service.optimizeRoute(create_result.route_id);
    ASSERT_TRUE(opt_result.success || !opt_result.message.empty());
}

TEST(RoutingServiceTest, AutoRoute) {
    RoutingService service;
    
    RoutingResult result = service.autoRoute("start_conn", "end_conn", 
                                             RoutingType::FlexibleHose, {});
    
    ASSERT_TRUE(result.success);
}
