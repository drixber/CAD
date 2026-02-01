#pragma once

#include <string>
#include <vector>
#include "ReferenceGeometry.h"

namespace cad {
namespace core {

enum class ConstraintType {
    Coincident,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Tangent,
    Equal,
    Distance,
    Angle,
    /** Symmetric: two entities symmetric about an axis (a, b = entity ids; value = axis angle in degrees, or 0 = Y-axis). */
    Symmetric,
    /** Fixed: entity position fixed (a = geometry id, b empty). */
    Fixed
};

struct Constraint {
    ConstraintType type{ConstraintType::Coincident};
    std::string a;
    std::string b;
    double value{0.0};
};

struct Parameter {
    std::string name;
    double value{0.0};
    std::string expression;
};

// Sketch geometry entities
enum class GeometryType {
    Point,
    Line,
    Circle,
    Arc,
    Rectangle,
    Ellipse,
    Polygon,
    Spline,
    Text
};

struct Point2D {
    double x{0.0};
    double y{0.0};
};

struct GeometryEntity {
    std::string id;
    GeometryType type{GeometryType::Point};
    Point2D start_point;
    Point2D end_point;
    Point2D center_point;
    double radius{0.0};       // circle/arc radius; ellipse major radius
    double start_angle{0.0};
    double end_angle{360.0};
    double width{0.0};       // rectangle width; ellipse minor radius
    double height{0.0};
    std::string text_content;  // for Text entity
};

class Sketch {
public:
    explicit Sketch(std::string name);

    const std::string& name() const;
    void addConstraint(const Constraint& constraint);
    const std::vector<Constraint>& constraints() const;
    void addParameter(const Parameter& parameter);
    std::vector<Parameter>& parameters();
    const std::vector<Parameter>& parameters() const;
    
    // Geometry operations
    std::string addLine(const Point2D& start, const Point2D& end);
    std::string addCircle(const Point2D& center, double radius);
    std::string addArc(const Point2D& center, double radius, double start_angle, double end_angle);
    std::string addRectangle(const Point2D& corner, double width, double height);
    std::string addPoint(const Point2D& point);
    std::string addEllipse(const Point2D& center, double radius_major, double radius_minor);
    std::string addPolygon(const std::vector<Point2D>& points);
    std::string addSpline(const std::vector<Point2D>& control_points);
    std::string addText(const Point2D& position, const std::string& text);

    const std::vector<GeometryEntity>& geometry() const;
    GeometryEntity* findGeometry(const std::string& id);
    const GeometryEntity* findGeometry(const std::string& id) const;
    bool removeGeometry(const std::string& id);

    /** 3D-Skizze (darius/SolidWorks): Kurven im Raum für Sweep/Pfadmuster. */
    void set3D(bool is_3d);
    bool is3D() const;
    std::string addWaypoint3D(double x, double y, double z);
    const std::vector<Point3D>& waypoints3D() const;

private:
    std::string name_;
    std::vector<Constraint> constraints_;
    std::vector<Parameter> parameters_;
    std::vector<GeometryEntity> geometry_;
    bool is_3d_{false};
    std::vector<Point3D> waypoints_3d_;
    int next_geometry_id_{1};
    
    std::string generateGeometryId();
};

}  // namespace core
}  // namespace cad
