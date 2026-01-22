#pragma once

#include <string>
#include <vector>

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
    Angle
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
    Rectangle
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
    double radius{0.0};
    double start_angle{0.0};
    double end_angle{360.0};
    double width{0.0};
    double height{0.0};
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
    
    const std::vector<GeometryEntity>& geometry() const;
    GeometryEntity* findGeometry(const std::string& id);
    const GeometryEntity* findGeometry(const std::string& id) const;
    bool removeGeometry(const std::string& id);

private:
    std::string name_;
    std::vector<Constraint> constraints_;
    std::vector<Parameter> parameters_;
    std::vector<GeometryEntity> geometry_;
    int next_geometry_id_{1};
    
    std::string generateGeometryId();
};

}  // namespace core
}  // namespace cad
