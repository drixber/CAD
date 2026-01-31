#include "ProjectFileService.h"

#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <cstdlib>
#include <map>
#include <set>

namespace cad {
namespace app {

ProjectFileService::ProjectFileService() = default;

bool ProjectFileService::saveProject(const std::string& file_path, const cad::core::Assembly& assembly) const {
    return saveProject(file_path, assembly, nullptr);
}

bool ProjectFileService::loadProject(const std::string& file_path, cad::core::Assembly& assembly) const {
    return loadProject(file_path, assembly, nullptr);
}

bool ProjectFileService::saveProject(const std::string& file_path, const cad::core::Assembly& assembly,
                                     const std::map<std::string, cad::core::Sketch>* sketches) const {
    try {
        std::ofstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        std::string data = serializeAssemblyWithSketches(assembly, sketches);
        file << "CADURSOR_PROJECT\n";
        file << "VERSION:2.0.0\n";
        file << "CREATED:" << getTimestamp() << "\n";
        file << "MODIFIED:" << getTimestamp() << "\n";
        file << "ASSEMBLY_DATA:\n";
        file << data;
        file << "END_PROJECT\n";
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool ProjectFileService::loadProject(const std::string& file_path, cad::core::Assembly& assembly,
                                     std::map<std::string, cad::core::Sketch>* sketches_out) const {
    try {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        std::string line;
        std::string assembly_data;
        bool in_assembly_data = false;
        while (std::getline(file, line)) {
            if (line == "ASSEMBLY_DATA:") {
                in_assembly_data = true;
                continue;
            }
            if (line == "END_PROJECT") {
                break;
            }
            if (in_assembly_data) {
                assembly_data += line + "\n";
            }
        }
        file.close();
        if (assembly_data.empty()) {
            return false;
        }
        if (sketches_out != nullptr) {
            return deserializeAssemblyWithSketches(assembly_data, assembly, sketches_out);
        }
        return deserializeAssembly(assembly_data, assembly);
    } catch (...) {
        return false;
    }
}

bool ProjectFileService::saveCheckpoint(const std::string& checkpoint_path, const cad::core::Assembly& assembly) const {
    // Checkpoint is same as project save, but with timestamp in filename
    return saveProject(checkpoint_path, assembly);
}

bool ProjectFileService::loadCheckpoint(const std::string& checkpoint_path, cad::core::Assembly& assembly) const {
    return loadProject(checkpoint_path, assembly);
}

ProjectFileInfo ProjectFileService::getProjectInfo(const std::string& file_path) const {
    ProjectFileInfo info;
    info.file_path = file_path;
    
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return info;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("VERSION:") == 0) {
                info.version = line.substr(8);
            } else if (line.find("CREATED:") == 0) {
                info.created_date = line.substr(8);
            } else if (line.find("MODIFIED:") == 0) {
                info.modified_date = line.substr(9);
            } else if (line.find("PART_IDS:") == 0) {
                std::string ids_str = line.substr(9);
                if (!ids_str.empty()) {
                    std::istringstream iss(ids_str);
                    std::string id;
                    while (std::getline(iss, id, ',')) {
                        info.part_ids.push_back(id);
                    }
                }
            }
        }
        
        file.close();
        
        // Extract project name from file path
        std::filesystem::path path(file_path);
        info.project_name = path.stem().string();
    } catch (...) {
        // Return default info on error
    }
    
    return info;
}

bool ProjectFileService::projectExists(const std::string& file_path) const {
    return std::filesystem::exists(file_path);
}

void ProjectFileService::enableAutoSave(bool enabled) {
    auto_save_enabled_ = enabled;
}

void ProjectFileService::setAutoSaveInterval(int seconds) {
    auto_save_interval_seconds_ = seconds > 0 ? seconds : 300;
}

void ProjectFileService::setAutoSavePath(const std::string& path) {
    auto_save_path_ = path;
    
    // Create directory if it doesn't exist
    try {
        std::filesystem::create_directories(path);
    } catch (...) {
        // Ignore errors
    }
}

bool ProjectFileService::isAutoSaveEnabled() const {
    return auto_save_enabled_;
}

bool ProjectFileService::triggerAutoSave(const cad::core::Assembly& assembly) const {
    if (!auto_save_enabled_) {
        return false;
    }
    
    try {
        // Cleanup old auto-saves before creating new one
        cleanupOldAutoSaves(7);
        
        // Create auto-save filename with timestamp
        std::string timestamp = getTimestamp();
        // Replace spaces and colons with underscores for filename safety
        std::string safe_timestamp = timestamp;
        std::replace(safe_timestamp.begin(), safe_timestamp.end(), ' ', '_');
        std::replace(safe_timestamp.begin(), safe_timestamp.end(), ':', '-');
        
        std::filesystem::path autosave_dir(auto_save_path_);
        if (!std::filesystem::exists(autosave_dir)) {
            std::filesystem::create_directories(autosave_dir);
        }
        
        std::string checkpoint_path = (autosave_dir / ("autosave_" + safe_timestamp + ".cadcheckpoint")).string();
        return saveCheckpoint(checkpoint_path, assembly);
    } catch (...) {
        return false;
    }
}

void ProjectFileService::cleanupOldAutoSaves(int days_to_keep) const {
    try {
        std::filesystem::path autosave_dir(auto_save_path_);
        if (!std::filesystem::exists(autosave_dir)) {
            return;
        }
        
        auto now = std::filesystem::file_time_type::clock::now();
        auto cutoff_time = now - std::chrono::hours(24 * days_to_keep);
        
        for (const auto& entry : std::filesystem::directory_iterator(autosave_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cadcheckpoint") {
                auto file_time = std::filesystem::last_write_time(entry.path());
                if (file_time < cutoff_time) {
                    std::filesystem::remove(entry.path());
                }
            }
        }
    } catch (...) {
        // Ignore cleanup errors
    }
}

std::vector<std::string> ProjectFileService::listCheckpoints(const std::string& project_path) const {
    std::vector<std::string> checkpoints;
    
    try {
        std::filesystem::path path(project_path);
        std::filesystem::path checkpoint_dir = path.parent_path() / "checkpoints";
        
        if (std::filesystem::exists(checkpoint_dir)) {
            for (const auto& entry : std::filesystem::directory_iterator(checkpoint_dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cadcheckpoint") {
                    checkpoints.push_back(entry.path().string());
                }
            }
        }
    } catch (...) {
        // Return empty list on error
    }
    
    return checkpoints;
}

bool ProjectFileService::deleteCheckpoint(const std::string& checkpoint_path) const {
    try {
        return std::filesystem::remove(checkpoint_path);
    } catch (...) {
        return false;
    }
}

std::string ProjectFileService::serializePart(const cad::core::Part& part) const {
    std::ostringstream oss;
    oss << "PART_FEATURES:" << part.features().size() << "\n";
    for (const auto& f : part.features()) {
        oss << "FEATURE_START\n";
        oss << "FNAME:" << f.name << "\n";
        oss << "FTYPE:" << static_cast<int>(f.type) << "\n";
        oss << "FSKETCH_ID:" << f.sketch_id << "\n";
        oss << "FDEPTH:" << f.depth << "\n";
        oss << "FSYMM:" << (f.symmetric ? 1 : 0) << "\n";
        oss << "FANGLE:" << f.angle << "\n";
        oss << "FAXIS:" << f.axis << "\n";
        oss << "FDIAM:" << f.diameter << "\n";
        oss << "FHOLE_DEPTH:" << f.hole_depth << "\n";
        oss << "FTHROUGH_ALL:" << (f.through_all ? 1 : 0) << "\n";
        oss << "FRADIUS:" << f.radius << "\n";
        oss << "FPATH_SKETCH:" << f.path_sketch_id << "\n";
        oss << "FTWIST:" << f.twist_angle << "\n";
        oss << "FSCALE:" << f.scale_factor << "\n";
        oss << "FPITCH:" << f.pitch << "\n";
        oss << "FREVOLUTIONS:" << f.revolutions << "\n";
        oss << "FCLOCKWISE:" << (f.clockwise ? 1 : 0) << "\n";
        oss << "FWALL:" << f.wall_thickness << "\n";
        oss << "FDRAFT_ANGLE:" << f.draft_angle << "\n";
        oss << "FDRAFT_PLANE:" << f.draft_plane << "\n";
        oss << "FMIRROR_PLANE:" << f.mirror_plane << "\n";
        oss << "FMERGE:" << (f.merge_result ? 1 : 0) << "\n";
        oss << "FTHREAD_STD:" << f.thread_standard << "\n";
        oss << "FTHREAD_PITCH:" << f.thread_pitch << "\n";
        oss << "FTHREAD_INTERNAL:" << (f.internal ? 1 : 0) << "\n";
        oss << "FRIB_THICK:" << f.rib_thickness << "\n";
        oss << "FRIB_PLANE:" << f.rib_plane << "\n";
        oss << "FCOUNT_X:" << f.count_x << "\n";
        oss << "FCOUNT_Y:" << f.count_y << "\n";
        oss << "FCOUNT_Z:" << f.count_z << "\n";
        oss << "FSPACING_X:" << f.spacing_x << "\n";
        oss << "FSPACING_Y:" << f.spacing_y << "\n";
        oss << "FSPACING_Z:" << f.spacing_z << "\n";
        oss << "FEDGE_IDS:";
        for (size_t i = 0; i < f.edge_ids.size(); ++i) {
            if (i > 0) oss << ",";
            oss << f.edge_ids[i];
        }
        oss << "\nFACE_IDS:";
        for (size_t i = 0; i < f.face_ids.size(); ++i) {
            if (i > 0) oss << ",";
            oss << f.face_ids[i];
        }
        oss << "\nFPARAMS:";
        for (const auto& p : f.parameters) {
            oss << p.first << "=" << p.second << ";";
        }
        oss << "\nFEATURE_END\n";
    }
    return oss.str();
}

bool ProjectFileService::deserializePart(const std::string& data, cad::core::Part& part) const {
    std::istringstream iss(data);
    std::string line;
    int feature_count = 0;
    cad::core::Feature current;
    bool in_feature = false;
    while (std::getline(iss, line)) {
        if (line.find("PART_FEATURES:") == 0) {
            feature_count = std::atoi(line.substr(14).c_str());
        } else if (line == "FEATURE_START") {
            in_feature = true;
            current = cad::core::Feature();
        } else if (line == "FEATURE_END") {
            if (in_feature) {
                part.addFeature(current);
                in_feature = false;
            }
        } else if (in_feature) {
            if (line.find("FNAME:") == 0) current.name = line.substr(6);
            else if (line.find("FTYPE:") == 0) current.type = static_cast<cad::core::FeatureType>(std::atoi(line.substr(6).c_str()));
            else if (line.find("FSKETCH_ID:") == 0) current.sketch_id = line.substr(11);
            else if (line.find("FDEPTH:") == 0) current.depth = std::stod(line.substr(7));
            else if (line.find("FSYMM:") == 0) current.symmetric = (line.substr(6) == "1");
            else if (line.find("FANGLE:") == 0) current.angle = std::stod(line.substr(7));
            else if (line.find("FAXIS:") == 0) current.axis = line.substr(6);
            else if (line.find("FDIAM:") == 0) current.diameter = std::stod(line.substr(6));
            else if (line.find("FHOLE_DEPTH:") == 0) current.hole_depth = std::stod(line.substr(12));
            else if (line.find("FTHROUGH_ALL:") == 0) current.through_all = (line.substr(13) == "1");
            else if (line.find("FRADIUS:") == 0) current.radius = std::stod(line.substr(8));
            else if (line.find("FPATH_SKETCH:") == 0) current.path_sketch_id = line.substr(13);
            else if (line.find("FTWIST:") == 0) current.twist_angle = std::stod(line.substr(7));
            else if (line.find("FSCALE:") == 0) current.scale_factor = std::stod(line.substr(7));
            else if (line.find("FPITCH:") == 0) current.pitch = std::stod(line.substr(7));
            else if (line.find("FREVOLUTIONS:") == 0) current.revolutions = std::stod(line.substr(13));
            else if (line.find("FCLOCKWISE:") == 0) current.clockwise = (line.substr(11) == "1");
            else if (line.find("FWALL:") == 0) current.wall_thickness = std::stod(line.substr(6));
            else if (line.find("FDRAFT_ANGLE:") == 0) current.draft_angle = std::stod(line.substr(13));
            else if (line.find("FDRAFT_PLANE:") == 0) current.draft_plane = line.substr(13);
            else if (line.find("FMIRROR_PLANE:") == 0) current.mirror_plane = line.substr(14);
            else if (line.find("FMERGE:") == 0) current.merge_result = (line.substr(7) == "1");
            else if (line.find("FTHREAD_STD:") == 0) current.thread_standard = line.substr(12);
            else if (line.find("FTHREAD_PITCH:") == 0) current.thread_pitch = std::stod(line.substr(14));
            else if (line.find("FTHREAD_INTERNAL:") == 0) current.internal = (line.substr(17) == "1");
            else if (line.find("FRIB_THICK:") == 0) current.rib_thickness = std::stod(line.substr(11));
            else if (line.find("FRIB_PLANE:") == 0) current.rib_plane = line.substr(11);
            else if (line.find("FCOUNT_X:") == 0) current.count_x = std::atoi(line.substr(9).c_str());
            else if (line.find("FCOUNT_Y:") == 0) current.count_y = std::atoi(line.substr(9).c_str());
            else if (line.find("FCOUNT_Z:") == 0) current.count_z = std::atoi(line.substr(9).c_str());
            else if (line.find("FSPACING_X:") == 0) current.spacing_x = std::stod(line.substr(11));
            else if (line.find("FSPACING_Y:") == 0) current.spacing_y = std::stod(line.substr(11));
            else if (line.find("FSPACING_Z:") == 0) current.spacing_z = std::stod(line.substr(11));
            else if (line.find("FEDGE_IDS:") == 0) {
                std::string ids = line.substr(10);
                if (!ids.empty()) {
                    std::istringstream ss(ids);
                    std::string id;
                    while (std::getline(ss, id, ',')) { current.edge_ids.push_back(id); }
                }
            } else if (line.find("FACE_IDS:") == 0) {
                std::string ids = line.substr(9);
                if (!ids.empty()) {
                    std::istringstream ss(ids);
                    std::string id;
                    while (std::getline(ss, id, ',')) { current.face_ids.push_back(id); }
                }
            } else if (line.find("FPARAMS:") == 0) {
                std::string params = line.substr(8);
                if (!params.empty()) {
                    std::istringstream ss(params);
                    std::string kv;
                    while (std::getline(ss, kv, ';')) {
                        size_t eq = kv.find('=');
                        if (eq != std::string::npos) {
                            current.parameters[kv.substr(0, eq)] = std::stod(kv.substr(eq + 1));
                        }
                    }
                }
            }
        }
    }
    return true;
}

std::string ProjectFileService::serializeSketch(const cad::core::Sketch& sketch) const {
    std::ostringstream oss;
    oss << "SKETCH_NAME:" << sketch.name() << "\n";
    oss << "GEOMETRY:" << sketch.geometry().size() << "\n";
    for (const auto& g : sketch.geometry()) {
        oss << "G:" << g.id << "|" << static_cast<int>(g.type) << "|"
            << g.start_point.x << "," << g.start_point.y << "|"
            << g.end_point.x << "," << g.end_point.y << "|"
            << g.center_point.x << "," << g.center_point.y << "|"
            << g.radius << "|" << g.start_angle << "|" << g.end_angle << "|"
            << g.width << "|" << g.height << "\n";
    }
    oss << "CONSTRAINTS:" << sketch.constraints().size() << "\n";
    for (const auto& c : sketch.constraints()) {
        oss << "C:" << static_cast<int>(c.type) << "|" << c.a << "|" << c.b << "|" << c.value << "\n";
    }
    oss << "PARAMETERS:" << sketch.parameters().size() << "\n";
    for (const auto& p : sketch.parameters()) {
        oss << "P:" << p.name << "|" << p.value << "|" << p.expression << "\n";
    }
    return oss.str();
}

namespace {
    double parseDouble(const std::string& s, double def = 0.0) {
        if (s.empty()) return def;
        try { return std::stod(s); } catch (...) { return def; }
    }
}

bool ProjectFileService::deserializeSketch(const std::string& data, cad::core::Sketch& sketch) const {
    std::istringstream iss(data);
    std::string line;
    std::map<std::string, std::string> old_to_new_id;
    while (std::getline(iss, line)) {
        if (line.find("SKETCH_NAME:") == 0) {
            std::string sketch_name = line.substr(12);
            sketch = cad::core::Sketch(sketch_name);
        } else if (line.find("G:") == 0) {
            std::string rest = line.substr(2);
            size_t p = 0, q;
            q = rest.find('|'); if (q == std::string::npos) continue;
            std::string old_id = rest.substr(p, q - p);
            p = q + 1; q = rest.find('|', p);
            int type_val = std::atoi(rest.substr(p, q - p).c_str());
            cad::core::GeometryType gtype = static_cast<cad::core::GeometryType>(type_val);
            auto next = [&]() {
                if (q == std::string::npos) return std::string();
                p = q + 1;
                q = rest.find('|', p);
                return rest.substr(p, (q == std::string::npos ? rest.size() : q) - p);
            };
            std::string pt_s = next(), pt_e = next(), pt_c = next();
            std::string r = next(), sa = next(), ea = next(), w = next(), h = next();
            auto parsePt = [](const std::string& pt, double& x, double& y) {
                size_t c = pt.find(',');
                if (c != std::string::npos) {
                    x = parseDouble(pt.substr(0, c));
                    y = parseDouble(pt.substr(c + 1));
                }
            };
            cad::core::Point2D start_pt{}, end_pt{}, center_pt{};
            parsePt(pt_s, start_pt.x, start_pt.y);
            parsePt(pt_e, end_pt.x, end_pt.y);
            parsePt(pt_c, center_pt.x, center_pt.y);
            double radius = parseDouble(r), start_angle = parseDouble(sa), end_angle = parseDouble(ea);
            double width = parseDouble(w), height = parseDouble(h);
            std::string new_id;
            switch (gtype) {
                case cad::core::GeometryType::Line:
                    new_id = sketch.addLine(start_pt, end_pt);
                    break;
                case cad::core::GeometryType::Circle:
                    new_id = sketch.addCircle(center_pt, radius);
                    break;
                case cad::core::GeometryType::Arc:
                    new_id = sketch.addArc(center_pt, radius, start_angle, end_angle);
                    break;
                case cad::core::GeometryType::Rectangle:
                    new_id = sketch.addRectangle(start_pt, width, height);
                    break;
                case cad::core::GeometryType::Point:
                    new_id = sketch.addPoint(start_pt);
                    break;
            }
            if (!new_id.empty()) old_to_new_id[old_id] = new_id;
        } else if (line.find("C:") == 0) {
            cad::core::Constraint c;
            std::string rest = line.substr(2);
            size_t p = 0, q = rest.find('|');
            if (q == std::string::npos) continue;
            c.type = static_cast<cad::core::ConstraintType>(std::atoi(rest.substr(p, q - p).c_str()));
            p = q + 1; q = rest.find('|', p);
            c.a = rest.substr(p, q - p);
            auto it_a = old_to_new_id.find(c.a);
            if (it_a != old_to_new_id.end()) c.a = it_a->second;
            p = q + 1; q = rest.find('|', p);
            c.b = rest.substr(p, q - p);
            auto it_b = old_to_new_id.find(c.b);
            if (it_b != old_to_new_id.end()) c.b = it_b->second;
            p = q + 1;
            c.value = parseDouble(rest.substr(p));
            sketch.addConstraint(c);
        } else if (line.find("P:") == 0) {
            cad::core::Parameter param;
            std::string rest = line.substr(2);
            size_t pos = 0, q = rest.find('|');
            if (q == std::string::npos) continue;
            param.name = rest.substr(pos, q - pos);
            pos = q + 1; q = rest.find('|', pos);
            param.value = parseDouble(rest.substr(pos, q - pos));
            pos = q + 1;
            param.expression = (pos < rest.size()) ? rest.substr(pos) : "";
            sketch.addParameter(param);
        }
    }
    return true;
}

std::string ProjectFileService::serializeAssembly(const cad::core::Assembly& assembly) const {
    std::ostringstream oss;
    
    oss << "COMPONENTS:" << assembly.components().size() << "\n";
    std::vector<std::string> part_ids;
    
    for (const auto& component : assembly.components()) {
        std::string part_id = component.part.name();
        part_ids.push_back(part_id);
        oss << "COMPONENT_ID:" << component.id << "\n";
        oss << "COMPONENT:" << part_id << "\n";
        oss << "TRANSFORM:" << component.transform.tx << ","
            << component.transform.ty << "," << component.transform.tz << "\n";
        oss << serializePart(component.part);
    }
    
    oss << "MATES:" << assembly.mates().size() << "\n";
    for (const auto& mate : assembly.mates()) {
        oss << "MATE:" << mate.component_a << "," << mate.component_b << ","
            << static_cast<int>(mate.type) << "," << mate.value << "\n";
    }
    
    oss << "PART_IDS:";
    for (size_t i = 0; i < part_ids.size(); ++i) {
        if (i > 0) oss << ",";
        oss << part_ids[i];
    }
    oss << "\n";
    
    return oss.str();
}

std::string ProjectFileService::serializeAssemblyWithSketches(const cad::core::Assembly& assembly,
                                                             const std::map<std::string, cad::core::Sketch>* sketches) const {
    std::string data = serializeAssembly(assembly);
    if (sketches != nullptr && !sketches->empty()) {
        std::set<std::string> sketch_ids;
        for (const auto& component : assembly.components()) {
            for (const auto& f : component.part.features()) {
                if (!f.sketch_id.empty()) sketch_ids.insert(f.sketch_id);
            }
        }
        data += "SKETCH_BLOCKS:\n";
        for (const std::string& sid : sketch_ids) {
            auto it = sketches->find(sid);
            if (it == sketches->end()) continue;
            data += "SKETCH_BLOCK_START\n";
            data += sid + "\n";
            data += serializeSketch(it->second);
            data += "SKETCH_BLOCK_END\n";
        }
    }
    return data;
}

bool ProjectFileService::deserializeAssemblyWithSketches(const std::string& data, cad::core::Assembly& assembly,
                                                         std::map<std::string, cad::core::Sketch>* sketches_out) const {
    const std::string marker = "SKETCH_BLOCKS:\n";
    size_t pos = data.find(marker);
    std::string assembly_data = (pos != std::string::npos) ? data.substr(0, pos) : data;
    std::string sketch_data = (pos != std::string::npos && pos + marker.size() < data.size())
                              ? data.substr(pos + marker.size()) : "";
    if (!deserializeAssembly(assembly_data, assembly)) {
        return false;
    }
    if (sketches_out == nullptr || sketch_data.empty()) {
        return true;
    }
    std::istringstream iss(sketch_data);
    std::string line;
    while (std::getline(iss, line)) {
        if (line == "SKETCH_BLOCK_START") {
            if (!std::getline(iss, line)) break;
            std::string sketch_id = line;
            std::string block;
            while (std::getline(iss, line) && line != "SKETCH_BLOCK_END") {
                block += line + "\n";
            }
            cad::core::Sketch sketch("");

            if (deserializeSketch(block, sketch)) {
                sketches_out->emplace(sketch_id, std::move(sketch));
            }
        }
    }
    return true;
}

bool ProjectFileService::deserializeAssembly(const std::string& data, cad::core::Assembly& assembly) const {
    try {
        // In real implementation, this would properly deserialize
        // For now, we parse basic structure and create assembly
        assembly = cad::core::Assembly();
        
        std::istringstream iss(data);
        std::string line;
        int component_count = 0;
        int mate_count = 0;
        
        while (std::getline(iss, line)) {
            if (line.find("COMPONENTS:") == 0) {
                component_count = std::stoi(line.substr(11));
            } else if (line.find("COMPONENT_ID:") == 0) {
                // Component ID (stored but not used in current implementation)
                std::getline(iss, line);  // Skip to next line (COMPONENT:)
            } else if (line.find("COMPONENT:") == 0) {
                std::string part_name = line.substr(10);
                cad::core::Part part(part_name);
                cad::core::Transform transform;
                if (std::getline(iss, line) && line.find("TRANSFORM:") == 0) {
                    std::string transform_str = line.substr(10);
                    size_t pos1 = transform_str.find(',');
                    size_t pos2 = transform_str.find(',', pos1 + 1);
                    if (pos1 != std::string::npos && pos2 != std::string::npos) {
                        transform.tx = std::stod(transform_str.substr(0, pos1));
                        transform.ty = std::stod(transform_str.substr(pos1 + 1, pos2 - pos1 - 1));
                        transform.tz = std::stod(transform_str.substr(pos2 + 1));
                    }
                }
                std::string part_data;
                if (std::getline(iss, line) && line.find("PART_FEATURES:") == 0) {
                    int n_features = std::atoi(line.substr(14).c_str());
                    part_data = line + "\n";
                    int features_read = 0;
                    while (features_read < n_features && std::getline(iss, line)) {
                        part_data += line + "\n";
                        if (line == "FEATURE_END") ++features_read;
                    }
                    deserializePart(part_data, part);
                }
                assembly.addComponent(part, transform);
            } else if (line.find("MATES:") == 0) {
                mate_count = std::stoi(line.substr(6));
            } else if (line.find("MATE:") == 0) {
                std::string mate_str = line.substr(5);
                size_t pos1 = mate_str.find(',');
                size_t pos2 = mate_str.find(',', pos1 + 1);
                size_t pos3 = mate_str.find(',', pos2 + 1);
                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    std::uint64_t id1 = std::stoull(mate_str.substr(0, pos1));
                    std::uint64_t id2 = std::stoull(mate_str.substr(pos1 + 1, pos2 - pos1 - 1));
                    int type = std::stoi(mate_str.substr(pos2 + 1, (pos3 != std::string::npos ? pos3 - pos2 - 1 : std::string::npos)));
                    double value = 0.0;
                    if (pos3 != std::string::npos) {
                        value = std::stod(mate_str.substr(pos3 + 1));
                    }
                    assembly.addMate({id1, id2, static_cast<cad::core::MateType>(type), value});
                }
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

std::string ProjectFileService::getTimestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

}  // namespace app
}  // namespace cad
