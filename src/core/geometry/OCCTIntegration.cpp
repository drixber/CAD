#include "OCCTIntegration.h"

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
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
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
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#endif

#include <sstream>
#include <iomanip>
#include <functional>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cad {
namespace core {
namespace geometry {

OCCTIntegration::OCCTIntegration() {
}

OCCTIntegration::~OCCTIntegration() {
    shutdown();
}

bool OCCTIntegration::initialize() {
#ifdef CAD_USE_OCCT
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    return true;
#else
    initialized_ = true;
    return true;
#endif
}

void OCCTIntegration::shutdown() {
    shape_cache_.clear();
    initialized_ = false;
}

#ifdef CAD_USE_OCCT
std::string OCCTIntegration::generateShapeId() {
    static int counter = 0;
    std::ostringstream oss;
    oss << "occt_shape_" << std::hex << std::setfill('0') << std::setw(8) << counter++;
    return oss.str();
}

OCCTShape OCCTIntegration::createBox(double width, double height, double depth) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    gp_Pnt corner(0, 0, 0);
    BRepPrimAPI_MakeBox box_maker(corner, width, height, depth);
    box_maker.Build();
    
    if (box_maker.IsDone()) {
        shape.shape = box_maker.Solid();
        shape.valid = true;
        shape_cache_[shape.id] = shape;
    }
    
    return shape;
}

OCCTShape OCCTIntegration::createCylinder(double radius, double height) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    gp_Ax2 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    BRepPrimAPI_MakeCylinder cylinder_maker(axis, radius, height);
    cylinder_maker.Build();
    
    if (cylinder_maker.IsDone()) {
        shape.shape = cylinder_maker.Solid();
        shape.valid = true;
        shape_cache_[shape.id] = shape;
    }
    
    return shape;
}

OCCTShape OCCTIntegration::createSphere(double radius) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    gp_Pnt center(0, 0, 0);
    BRepPrimAPI_MakeSphere sphere_maker(center, radius);
    sphere_maker.Build();
    
    if (sphere_maker.IsDone()) {
        shape.shape = sphere_maker.Solid();
        shape.valid = true;
        shape_cache_[shape.id] = shape;
    }
    
    return shape;
}

OCCTShape OCCTIntegration::extrudeProfile(const std::vector<std::pair<double, double>>& profile, double depth) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    if (profile.size() < 3) {
        return shape;
    }
    
    BRepBuilderAPI_MakeWire wire_maker;
    for (size_t i = 0; i < profile.size(); ++i) {
        gp_Pnt pnt(profile[i].first, profile[i].second, 0.0);
        BRepBuilderAPI_MakeVertex vertex_maker(pnt);
        TopoDS_Vertex vertex = vertex_maker.Vertex();
        
        if (i > 0) {
            gp_Pnt prev_pnt(profile[i-1].first, profile[i-1].second, 0.0);
            BRepBuilderAPI_MakeEdge edge_maker(prev_pnt, pnt);
            if (edge_maker.IsDone()) {
                wire_maker.Add(edge_maker.Edge());
            }
        }
    }
    
    if (wire_maker.IsDone()) {
        BRepBuilderAPI_MakeFace face_maker(wire_maker.Wire());
        if (face_maker.IsDone()) {
            TopoDS_Face face = face_maker.Face();
            gp_Vec extrude_dir(0, 0, depth);
            BRepPrimAPI_MakePrism prism_maker(face, extrude_dir);
            if (prism_maker.IsDone()) {
                shape.shape = prism_maker.Shape();
                shape.valid = true;
                shape_cache_[shape.id] = shape;
            }
        }
    }
    
    return shape;
}

OCCTShape OCCTIntegration::revolveProfile(const std::vector<std::pair<double, double>>& profile, double angle) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    if (profile.size() < 3) {
        return shape;
    }
    
    BRepBuilderAPI_MakeWire wire_maker;
    for (size_t i = 0; i < profile.size(); ++i) {
        gp_Pnt pnt(profile[i].first, profile[i].second, 0.0);
        if (i > 0) {
            gp_Pnt prev_pnt(profile[i-1].first, profile[i-1].second, 0.0);
            BRepBuilderAPI_MakeEdge edge_maker(prev_pnt, pnt);
            if (edge_maker.IsDone()) {
                wire_maker.Add(edge_maker.Edge());
            }
        }
    }
    
    if (wire_maker.IsDone()) {
        BRepBuilderAPI_MakeFace face_maker(wire_maker.Wire());
        if (face_maker.IsDone()) {
            TopoDS_Face face = face_maker.Face();
            gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
            BRepPrimAPI_MakeRevol revolve_maker(face, axis, angle * M_PI / 180.0);
            if (revolve_maker.IsDone()) {
                shape.shape = revolve_maker.Shape();
                shape.valid = true;
                shape_cache_[shape.id] = shape;
            }
        }
    }
    
    return shape;
}

OCCTShape OCCTIntegration::transformShape(const OCCTShape& shape, const double* transform_matrix) {
    OCCTShape result;
    result.id = generateShapeId();
    
    if (!shape.valid || !transform_matrix) {
        return result;
    }
    
    gp_Trsf transform;
    gp_Mat rotation_matrix;
    rotation_matrix.SetValue(1, 1, transform_matrix[0]);
    rotation_matrix.SetValue(1, 2, transform_matrix[1]);
    rotation_matrix.SetValue(1, 3, transform_matrix[2]);
    rotation_matrix.SetValue(2, 1, transform_matrix[4]);
    rotation_matrix.SetValue(2, 2, transform_matrix[5]);
    rotation_matrix.SetValue(2, 3, transform_matrix[6]);
    rotation_matrix.SetValue(3, 1, transform_matrix[8]);
    rotation_matrix.SetValue(3, 2, transform_matrix[9]);
    rotation_matrix.SetValue(3, 3, transform_matrix[10]);
    
    gp_Vec translation(transform_matrix[12], transform_matrix[13], transform_matrix[14]);
    transform.SetTransformation(rotation_matrix, translation);
    
    BRepBuilderAPI_Transform transform_maker(shape.shape, transform);
    result.shape = transform_maker.Shape();
    result.valid = true;
    shape_cache_[result.id] = result;
    
    return result;
}

OCCTShape OCCTIntegration::booleanUnion(const OCCTShape& shape1, const OCCTShape& shape2) {
    OCCTShape result;
    result.id = generateShapeId();
    
    if (!shape1.valid || !shape2.valid) {
        return result;
    }
    
    BRepAlgoAPI_Fuse fuse_maker(shape1.shape, shape2.shape);
    if (fuse_maker.IsDone()) {
        result.shape = fuse_maker.Shape();
        result.valid = true;
        shape_cache_[result.id] = result;
    }
    
    return result;
}

OCCTShape OCCTIntegration::booleanSubtract(const OCCTShape& shape1, const OCCTShape& shape2) {
    OCCTShape result;
    result.id = generateShapeId();
    
    if (!shape1.valid || !shape2.valid) {
        return result;
    }
    
    BRepAlgoAPI_Cut cut_maker(shape1.shape, shape2.shape);
    if (cut_maker.IsDone()) {
        result.shape = cut_maker.Shape();
        result.valid = true;
        shape_cache_[result.id] = result;
    }
    
    return result;
}

OCCTShape OCCTIntegration::booleanIntersect(const OCCTShape& shape1, const OCCTShape& shape2) {
    OCCTShape result;
    result.id = generateShapeId();
    
    if (!shape1.valid || !shape2.valid) {
        return result;
    }
    
    BRepAlgoAPI_Common common_maker(shape1.shape, shape2.shape);
    if (common_maker.IsDone()) {
        result.shape = common_maker.Shape();
        result.valid = true;
        shape_cache_[result.id] = result;
    }
    
    return result;
}

bool OCCTIntegration::exportToStep(const OCCTShape& shape, const std::string& file_path) {
    if (!shape.valid) {
        return false;
    }
    
    STEPControl_Writer writer;
    IFSelect_ReturnStatus status = writer.Transfer(shape.shape, STEPControl_AsIs);
    if (status == IFSelect_RetDone) {
        status = writer.Write(file_path.c_str());
        return status == IFSelect_RetDone;
    }
    
    return false;
}

OCCTShape OCCTIntegration::importFromStep(const std::string& file_path) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(file_path.c_str());
    if (status == IFSelect_RetDone) {
        int num_shapes = reader.NbRootsForTransfer();
        if (num_shapes > 0) {
            reader.TransferRoot(1);
            int num_results = reader.NbShapes();
            if (num_results > 0) {
                shape.shape = reader.Shape(1);
                shape.valid = true;
                shape_cache_[shape.id] = shape;
            }
        }
    }
    
    return shape;
}

bool OCCTIntegration::exportToIges(const OCCTShape& shape, const std::string& file_path) {
    if (!shape.valid) {
        return false;
    }
    
    IGESControl_Writer writer;
    writer.AddShape(shape.shape);
    return writer.Write(file_path.c_str()) == Standard_True;
}

OCCTShape OCCTIntegration::importFromIges(const std::string& file_path) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    IGESControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(file_path.c_str());
    if (status == IFSelect_RetDone) {
        reader.TransferRoots();
        int num_shapes = reader.NbShapes();
        if (num_shapes > 0) {
            shape.shape = reader.Shape(1);
            shape.valid = true;
            shape_cache_[shape.id] = shape;
        }
    }
    
    return shape;
}

bool OCCTIntegration::exportToStl(const OCCTShape& shape, const std::string& file_path, bool ascii_mode) {
    if (!shape.valid) {
        return false;
    }
    
    BRepMesh_IncrementalMesh mesh_maker(shape.shape, 0.1);
    mesh_maker.Perform();
    
    StlAPI_Writer writer;
    return writer.Write(shape.shape, file_path.c_str()) == Standard_True;
}

OCCTShape OCCTIntegration::importFromStl(const std::string& file_path) {
    OCCTShape shape;
    shape.id = generateShapeId();
    
    StlAPI_Reader reader;
    TopoDS_Shape stl_shape;
    if (reader.Read(stl_shape, file_path.c_str()) == Standard_True) {
        shape.shape = stl_shape;
        shape.valid = true;
        shape_cache_[shape.id] = shape;
    }
    
    return shape;
}

void OCCTIntegration::getBoundingBox(const OCCTShape& shape, double& min_x, double& min_y, double& min_z,
                                     double& max_x, double& max_y, double& max_z) {
    if (!shape.valid) {
        return;
    }
    
    Bnd_Box bbox;
    BRepBndLib::Add(shape.shape, bbox);
    
    if (!bbox.IsVoid()) {
        double x_min, y_min, z_min, x_max, y_max, z_max;
        bbox.Get(x_min, y_min, z_min, x_max, y_max, z_max);
        min_x = x_min;
        min_y = y_min;
        min_z = z_min;
        max_x = x_max;
        max_y = y_max;
        max_z = z_max;
    }
}

double OCCTIntegration::calculateVolume(const OCCTShape& shape) {
    if (!shape.valid) {
        return 0.0;
    }
    
    GProp_GProps props;
    BRepGProp::VolumeProperties(shape.shape, props);
    return props.Mass();
}

double OCCTIntegration::calculateSurfaceArea(const OCCTShape& shape) {
    if (!shape.valid) {
        return 0.0;
    }
    
    GProp_GProps props;
    BRepGProp::SurfaceProperties(shape.shape, props);
    return props.Mass();
}
#endif

}  // namespace geometry
}  // namespace core
}  // namespace cad
