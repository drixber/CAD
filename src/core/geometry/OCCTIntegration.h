#pragma once

#ifdef CAD_USE_OCCT
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <StlAPI_Writer.hxx>
#include <StlAPI_Reader.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <TopTools_ListOfShape.hxx>
#endif

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace cad {
namespace core {
namespace geometry {

struct OCCTShape {
#ifdef CAD_USE_OCCT
    TopoDS_Shape shape;
#endif
    std::string id;
    bool valid{false};
};

class OCCTIntegration {
public:
    OCCTIntegration();
    ~OCCTIntegration();
    
    bool initialize();
    void shutdown();
    
#ifdef CAD_USE_OCCT
    OCCTShape createBox(double width, double height, double depth);
    OCCTShape createCylinder(double radius, double height);
    OCCTShape createSphere(double radius);
    OCCTShape extrudeProfile(const std::vector<std::pair<double, double>>& profile, double depth);
    OCCTShape revolveProfile(const std::vector<std::pair<double, double>>& profile, double angle);
    
    OCCTShape transformShape(const OCCTShape& shape, const double* transform_matrix);
    OCCTShape booleanUnion(const OCCTShape& shape1, const OCCTShape& shape2);
    OCCTShape booleanSubtract(const OCCTShape& shape1, const OCCTShape& shape2);
    OCCTShape booleanIntersect(const OCCTShape& shape1, const OCCTShape& shape2);
    
    bool exportToStep(const OCCTShape& shape, const std::string& file_path);
    OCCTShape importFromStep(const std::string& file_path);
    bool exportToIges(const OCCTShape& shape, const std::string& file_path);
    OCCTShape importFromIges(const std::string& file_path);
    bool exportToStl(const OCCTShape& shape, const std::string& file_path, bool ascii_mode);
    OCCTShape importFromStl(const std::string& file_path);
    
    void getBoundingBox(const OCCTShape& shape, double& min_x, double& min_y, double& min_z,
                       double& max_x, double& max_y, double& max_z);
    double calculateVolume(const OCCTShape& shape);
    double calculateSurfaceArea(const OCCTShape& shape);
    
    TopoDS_Shape getShape(const OCCTShape& occt_shape) const { return occt_shape.shape; }
#else
    OCCTShape createBox(double width, double height, double depth) { return {}; }
    OCCTShape createCylinder(double radius, double height) { return {}; }
    OCCTShape createSphere(double radius) { return {}; }
    OCCTShape extrudeProfile(const std::vector<std::pair<double, double>>& profile, double depth) { return {}; }
    OCCTShape revolveProfile(const std::vector<std::pair<double, double>>& profile, double angle) { return {}; }
    OCCTShape transformShape(const OCCTShape& shape, const double* transform_matrix) { return {}; }
    OCCTShape booleanUnion(const OCCTShape& shape1, const OCCTShape& shape2) { return {}; }
    OCCTShape booleanSubtract(const OCCTShape& shape1, const OCCTShape& shape2) { return {}; }
    OCCTShape booleanIntersect(const OCCTShape& shape1, const OCCTShape& shape2) { return {}; }
    bool exportToStep(const OCCTShape& shape, const std::string& file_path) { return false; }
    OCCTShape importFromStep(const std::string& file_path) { return {}; }
    bool exportToIges(const OCCTShape& shape, const std::string& file_path) { return false; }
    OCCTShape importFromIges(const std::string& file_path) { return {}; }
    bool exportToStl(const OCCTShape& shape, const std::string& file_path, bool ascii_mode) { return false; }
    OCCTShape importFromStl(const std::string& file_path) { return {}; }
    void getBoundingBox(const OCCTShape& shape, double& min_x, double& min_y, double& min_z,
                       double& max_x, double& max_y, double& max_z) {}
    double calculateVolume(const OCCTShape& shape) { return 0.0; }
    double calculateSurfaceArea(const OCCTShape& shape) { return 0.0; }
    void* getShape(const OCCTShape& occt_shape) const { return nullptr; }
#endif

private:
    bool initialized_{false};
    std::map<std::string, OCCTShape> shape_cache_;
    
#ifdef CAD_USE_OCCT
    std::string generateShapeId();
#endif
};

}  // namespace geometry
}  // namespace core
}  // namespace cad
