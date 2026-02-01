#include "topology/Solid.h"
#include "topology/Face.h"
#include "topology/Edge.h"
#include "topology/Vertex.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace cad {
namespace kernel {
namespace topology {

void Solid::setOuterShell(std::shared_ptr<Shell> shell) {
    outerShell_ = std::move(shell);
}

void Solid::addInnerShell(std::shared_ptr<Shell> shell) {
    innerShells_.push_back(std::move(shell));
}

double Solid::volume() const {
    if (!outerShell_ || outerShell_->faces().empty()) return 0.0;
    double vol = 0.0;
    for (const auto& f : outerShell_->faces()) {
        if (f->surface()) {
            const double uMid = (f->surface()->uMin() + f->surface()->uMax()) * 0.5;
            const double vMid = (f->surface()->vMin() + f->surface()->vMax()) * 0.5;
            math::Point3 p = f->surface()->pointAt(uMid, vMid);
            math::Vector3 n = f->surface()->normalAt(uMid, vMid);
            vol += p.x * n.x + p.y * n.y + p.z * n.z;
        }
    }
    return std::abs(vol) / 3.0;
}

void Solid::bounds(double& minX, double& minY, double& minZ, double& maxX, double& maxY, double& maxZ) const {
    minX = minY = minZ = std::numeric_limits<double>::max();
    maxX = maxY = maxZ = std::numeric_limits<double>::lowest();
    if (!outerShell_) return;
    for (const auto& face : outerShell_->faces()) {
        const Loop* loop = face->outerLoop();
        if (!loop || !loop->wire()) continue;
        for (const auto& edge : loop->wire()->edges()) {
            math::Point3 p0 = edge->startVertex()->point();
            math::Point3 p1 = edge->endVertex()->point();
            minX = std::min(minX, std::min(p0.x, p1.x));
            maxX = std::max(maxX, std::max(p0.x, p1.x));
            minY = std::min(minY, std::min(p0.y, p1.y));
            maxY = std::max(maxY, std::max(p0.y, p1.y));
            minZ = std::min(minZ, std::min(p0.z, p1.z));
            maxZ = std::max(maxZ, std::max(p0.z, p1.z));
        }
    }
    if (minX == std::numeric_limits<double>::max()) minX = maxX = minY = maxY = minZ = maxZ = 0.0;
}

}  // namespace topology
}  // namespace kernel
}  // namespace cad
