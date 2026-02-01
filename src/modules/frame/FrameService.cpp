#include "FrameService.h"
#include <cmath>
#include <algorithm>
#include <map>

namespace cad {
namespace modules {

static const double kEpsilon = 1e-6;

std::vector<FrameProfileSpec> FrameService::buildCatalog() const {
    std::vector<FrameProfileSpec> catalog;
    auto add = [&catalog](const std::string& id, const std::string& name, FrameProfileType t,
                           const std::string& standard, double w, double h, double wall = 0.0) {
        FrameProfileSpec s;
        s.id = id;
        s.name = name;
        s.type = t;
        s.standard = standard;
        s.width_mm = w;
        s.height_mm = h;
        s.wall_mm = wall;
        if (t == FrameProfileType::Rectangle)
            s.area_mm2 = w * h;
        else if (t == FrameProfileType::Tube)
            s.area_mm2 = 3.14159 * (w * 0.5) * (w * 0.5) - 3.14159 * (w * 0.5 - wall) * (w * 0.5 - wall);
        else
            s.area_mm2 = w * h - (w - 2 * wall) * (h - 2 * wall);
        catalog.push_back(s);
    };
    add("RECT_40x40", "Rechteck 40x40", FrameProfileType::Rectangle, "DIN", 40, 40);
    add("RECT_50x30", "Rechteck 50x30", FrameProfileType::Rectangle, "DIN", 50, 30);
    add("TUBE_30", "Rundrohr Ø30", FrameProfileType::Tube, "DIN", 30, 30, 2);
    add("TUBE_40", "Rundrohr Ø40", FrameProfileType::Tube, "DIN", 40, 40, 2.5);
    add("U_40x25", "U-Profil 40x25", FrameProfileType::U, "DIN", 40, 25, 2);
    add("I_80", "I-Träger 80", FrameProfileType::I, "DIN", 80, 80, 5);
    add("L_40x40", "L-Profil 40x40", FrameProfileType::L, "DIN", 40, 40, 4);
    return catalog;
}

std::vector<FrameProfileSpec> FrameService::getProfileCatalog() const {
    return buildCatalog();
}

FrameProfileSpec FrameService::getProfile(const std::string& profile_id) const {
    for (const auto& p : buildCatalog()) {
        if (p.id == profile_id) return p;
    }
    return {};
}

double FrameService::segmentLength(double x1, double y1, double z1, double x2, double y2, double z2) const {
    double dx = x2 - x1, dy = y2 - y1, dz = z2 - z1;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool FrameService::addMember(const std::string& assembly_or_frame_id, const FrameMember& member) {
    FrameMember m = member;
    if (m.member_id.empty()) {
        m.member_id = "M" + std::to_string(frame_members_[assembly_or_frame_id].size() + 1);
    }
    if (m.length_mm <= 0 && (m.start_x != m.end_x || m.start_y != m.end_y || m.start_z != m.end_z)) {
        m.length_mm = segmentLength(m.start_x, m.start_y, m.start_z, m.end_x, m.end_y, m.end_z);
    }
    frame_members_[assembly_or_frame_id].push_back(m);
    return true;
}

bool FrameService::addMemberAlongLine(const std::string& frame_id, const std::string& profile_id,
                                       double x1, double y1, double z1, double x2, double y2, double z2,
                                       bool miter, const std::string& group) {
    FrameMember m;
    m.profile_id = profile_id;
    m.start_x = x1; m.start_y = y1; m.start_z = z1;
    m.end_x = x2; m.end_y = y2; m.end_z = z2;
    m.length_mm = segmentLength(x1, y1, z1, x2, y2, z2);
    m.miter = miter;
    m.group = group;
    return addMember(frame_id, m);
}

std::vector<FrameMember> FrameService::getMembers(const std::string& frame_id) const {
    auto it = frame_members_.find(frame_id);
    if (it != frame_members_.end()) return it->second;
    return {};
}

void FrameService::setMiterAngle(double degrees) { miter_angle_deg_ = degrees; }
double FrameService::getMiterAngle() const { return miter_angle_deg_; }

std::vector<CutListEntry> FrameService::getCutList(const std::string& frame_id) const {
    std::vector<CutListEntry> cut_list;
    std::map<std::string, std::map<double, CutListEntry>> by_profile_length;
    auto members = getMembers(frame_id);
    auto catalog = buildCatalog();
    for (const auto& m : members) {
        double len = m.length_mm > 0 ? m.length_mm : segmentLength(m.start_x, m.start_y, m.start_z, m.end_x, m.end_y, m.end_z);
        std::string name = m.profile_id;
        for (const auto& p : catalog) {
            if (p.id == m.profile_id) { name = p.name; break; }
        }
        auto& by_len = by_profile_length[m.profile_id];
        double key = std::round(len * 10) / 10.0;
        if (by_len.find(key) == by_len.end()) {
            CutListEntry e;
            e.profile_id = m.profile_id;
            e.profile_name = name;
            e.length_mm = key;
            e.quantity = 0;
            by_len[key] = e;
        }
        by_len[key].quantity++;
        by_len[key].member_ids.push_back(m.member_id);
    }
    for (const auto& kv : by_profile_length) {
        for (const auto& kv2 : kv.second) {
            cut_list.push_back(kv2.second);
        }
    }
    return cut_list;
}

FrameService::OptimizationResult FrameService::optimizeLengths(const std::string& frame_id, int stock_length_mm) const {
    OptimizationResult result;
    result.stock_length_mm = stock_length_mm;
    auto cuts = getCutList(frame_id);
    for (const auto& c : cuts) {
        int need = c.quantity;
        double len = c.length_mm;
        while (need-- > 0) {
            int pieces = static_cast<int>(stock_length_mm / len);
            if (pieces < 1) pieces = 1;
            double waste = stock_length_mm - pieces * len;
            result.total_waste_mm += waste;
            CutListEntry e = c;
            e.quantity = 1;
            e.length_mm = len;
            result.ordered_cuts.push_back(e);
        }
    }
    return result;
}

}  // namespace modules
}  // namespace cad
