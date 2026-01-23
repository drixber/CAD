#include <gtest/gtest.h>
#include "core/FreeCAD/FreeCADAdapter.h"
#include "core/Modeler/Sketch.h"

using namespace cad::core;

TEST(FreeCADIntegrationTest, CreateSketch) {
    FreeCADAdapter adapter;
    adapter.initializeSession();
    adapter.createDocument("TestDoc");
    
    bool result = adapter.createSketch("TestSketch", "XY");
    
    ASSERT_TRUE(result || !adapter.isAvailable());
}

TEST(FreeCADIntegrationTest, CreateDrawing) {
    FreeCADAdapter adapter;
    adapter.initializeSession();
    adapter.createDocument("TestDoc");
    
    bool result = adapter.createDrawing("TestDrawing", "A4_Landscape");
    
    ASSERT_TRUE(result || !adapter.isAvailable());
}

TEST(FreeCADIntegrationTest, SyncSketch) {
    FreeCADAdapter adapter;
    adapter.initializeSession();
    adapter.createDocument("TestDoc");
    
    Sketch sketch("TestSketch");
    bool result = adapter.syncSketch(sketch);
    
    ASSERT_TRUE(result || !adapter.isAvailable());
}
