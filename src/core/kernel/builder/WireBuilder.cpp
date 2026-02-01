#include "builder/WireBuilder.h"
#include "geometry2d/Line2D.h"
#include "geometry2d/Circle2D.h"
#include "geometry2d/Arc2D.h"
#include "geometry2d/Spline2D.h"
#include "core/Modeler/Sketch.h"
#include <cmath>
#include <memory>

namespace cad {
namespace kernel {
namespace builder {

static const double deg2rad = 3.14159265358979323846 / 180.0;

geometry2d::Wire2D WireBuilder::build(const cad::core::Sketch& sketch) {
    geometry2d::Wire2D wire;
    const auto& geom = sketch.geometry();
    for (const auto& g : geom) {
        using GT = cad::core::GeometryType;
        math::Point2 c(g.center_point.x, g.center_point.y);
        math::Point2 s(g.start_point.x, g.start_point.y);
        math::Point2 e(g.end_point.x, g.end_point.y);
        switch (g.type) {
            case GT::Line:
                wire.add(std::make_shared<geometry2d::Line2D>(s, e));
                break;
            case GT::Circle:
                wire.add(std::make_shared<geometry2d::Circle2D>(c, g.radius));
                break;
            case GT::Arc:
                wire.add(std::make_shared<geometry2d::Arc2D>(c, g.radius,
                    g.start_angle * deg2rad, g.end_angle * deg2rad));
                break;
            case GT::Rectangle:
                wire.add(std::make_shared<geometry2d::Line2D>(s, math::Point2(s.x + g.width, s.y)));
                wire.add(std::make_shared<geometry2d::Line2D>(math::Point2(s.x + g.width, s.y), math::Point2(s.x + g.width, s.y + g.height)));
                wire.add(std::make_shared<geometry2d::Line2D>(math::Point2(s.x + g.width, s.y + g.height), math::Point2(s.x, s.y + g.height)));
                wire.add(std::make_shared<geometry2d::Line2D>(math::Point2(s.x, s.y + g.height), s));
                break;
            case GT::Ellipse:
            case GT::Polygon:
            case GT::Spline: {
                std::vector<math::Point2> pts;
                pts.push_back(s);
                if (g.type == GT::Spline || g.type == GT::Polygon) pts.push_back(e);
                if (pts.size() >= 2)
                    wire.add(std::make_shared<geometry2d::Spline2D>(pts));
                break;
            }
            case GT::Point:
            case GT::Text:
            default:
                break;
        }
    }
    return wire;
}

std::vector<geometry2d::Wire2D> WireBuilder::buildWires(const cad::core::Sketch& sketch) {
    std::vector<geometry2d::Wire2D> wires;
    wires.push_back(build(sketch));
    return wires;
}

}  // namespace builder
}  // namespace kernel
}  // namespace cad
