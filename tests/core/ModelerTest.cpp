#include <cassert>
#include <iostream>
#include "core/Modeler/Modeler.h"
#include "core/Modeler/Sketch.h"
#include "core/Modeler/Part.h"

using namespace cad::core;

void testSketchGeometry() {
    std::cout << "Testing Sketch Geometry..." << std::endl;
    
    Sketch sketch("TestSketch");
    
    // Test adding geometry
    Point2D start{0.0, 0.0};
    Point2D end{10.0, 10.0};
    std::string line_id = sketch.addLine(start, end);
    assert(!line_id.empty());
    assert(sketch.geometry().size() == 1);
    
    Point2D center{5.0, 5.0};
    std::string circle_id = sketch.addCircle(center, 5.0);
    assert(!circle_id.empty());
    assert(sketch.geometry().size() == 2);
    
    std::string rect_id = sketch.addRectangle({0.0, 0.0}, 10.0, 10.0);
    assert(!rect_id.empty());
    assert(sketch.geometry().size() == 3);
    
    // Test finding geometry
    const GeometryEntity* found = sketch.findGeometry(line_id);
    assert(found != nullptr);
    assert(found->type == GeometryType::Line);
    
    // Test removing geometry
    bool removed = sketch.removeGeometry(circle_id);
    assert(removed);
    assert(sketch.geometry().size() == 2);
    
    std::cout << "  ✓ Sketch Geometry tests passed" << std::endl;
}

void testPartFeatures() {
    std::cout << "Testing Part Features..." << std::endl;
    
    Sketch sketch("TestSketch");
    sketch.addLine({0.0, 0.0}, {10.0, 10.0});
    
    Modeler modeler;
    Part part = modeler.createPart(sketch);
    
    // Test creating features
    std::string extrude_id = part.createExtrude(sketch.name(), 10.0);
    assert(!extrude_id.empty());
    assert(part.features().size() == 1);
    
    std::string revolve_id = part.createRevolve(sketch.name(), 360.0);
    assert(!revolve_id.empty());
    assert(part.features().size() == 2);
    
    std::string hole_id = part.createHole(5.0, 10.0);
    assert(!hole_id.empty());
    assert(part.features().size() == 3);
    
    // Test finding features
    const Feature* found = part.findFeature(extrude_id);
    assert(found != nullptr);
    assert(found->type == FeatureType::Extrude);
    assert(found->depth == 10.0);
    
    // Test removing features
    bool removed = part.removeFeature(revolve_id);
    assert(removed);
    assert(part.features().size() == 2);
    
    std::cout << "  ✓ Part Features tests passed" << std::endl;
}

void testAssemblyMates() {
    std::cout << "Testing Assembly Mates..." << std::endl;
    
    Assembly assembly;
    Part partA("PartA");
    Part partB("PartB");
    
    Transform transformA;
    Transform transformB;
    transformB.tx = 10.0;
    
    std::uint64_t idA = assembly.addComponent(partA, transformA);
    std::uint64_t idB = assembly.addComponent(partB, transformB);
    
    assert(assembly.components().size() == 2);
    
    // Test creating mates
    std::string mate_name = assembly.createMate(idA, idB, 5.0);
    assert(!mate_name.empty());
    assert(assembly.mates().size() == 1);
    
    std::string flush_name = assembly.createFlush(idA, idB);
    assert(!flush_name.empty());
    assert(assembly.mates().size() == 2);
    
    std::string angle_name = assembly.createAngle(idA, idB, 45.0);
    assert(!angle_name.empty());
    assert(assembly.mates().size() == 3);
    
    // Test solving mates
    bool solved = assembly.solveMates();
    assert(solved);
    
    // Verify transforms were updated
    const AssemblyComponent* compB = assembly.findComponent(idB);
    assert(compB != nullptr);
    
    std::cout << "  ✓ Assembly Mates tests passed" << std::endl;
}

void testConstraintSolver() {
    std::cout << "Testing Constraint Solver..." << std::endl;
    
    Sketch sketch("TestSketch");
    Point2D p1{0.0, 0.0};
    Point2D p2{10.0, 0.0};
    std::string line1_id = sketch.addLine(p1, p2);
    
    Point2D p3{0.0, 10.0};
    Point2D p4{10.0, 10.0};
    std::string line2_id = sketch.addLine(p3, p4);
    
    // Add constraints
    sketch.addConstraint({ConstraintType::Distance, line1_id, line2_id, 10.0});
    sketch.addConstraint({ConstraintType::Horizontal, line1_id, "", 0.0});
    
    Modeler modeler;
    bool solved = modeler.solveConstraints(sketch);
    assert(solved);
    
    std::cout << "  ✓ Constraint Solver tests passed" << std::endl;
}

int main() {
    std::cout << "Running Core Modeler Tests..." << std::endl;
    std::cout << std::endl;
    
    try {
        testSketchGeometry();
        testPartFeatures();
        testAssemblyMates();
        testConstraintSolver();
        
        std::cout << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}

