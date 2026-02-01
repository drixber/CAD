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

std::string ProjectFileService::projectConfigPathForProject(const std::string& project_file_path) {
    std::filesystem::path p(project_file_path);
    return (p.parent_path() / (p.stem().string() + ".hcadproject")).string();
}

static std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> out;
    std::string cell;
    for (size_t i = 0; i <= line.size(); ++i) {
        char c = (i < line.size()) ? line[i] : ',';
        if (c == ',') {
            while (!cell.empty() && (cell.front() == '"' || cell.back() == ' ')) {
                if (cell.front() == '"') cell.erase(0, 1);
                if (!cell.empty() && cell.back() == '"') cell.pop_back();
            }
            out.push_back(cell);
            cell.clear();
        } else {
            cell += c;
        }
    }
    return out;
}

bool ProjectFileService::loadPartConfigurationsFromCsv(cad::core::Part& part, const std::string& csv_path) const {
    try {
        if (csv_path.empty()) return false;
        std::ifstream file(csv_path);
        if (!file.is_open()) return false;
        std::string header_line;
        if (!std::getline(file, header_line) || header_line.empty()) return false;
        std::vector<std::string> headers = splitCsvLine(header_line);
        if (headers.empty() || headers[0] != "Name") return false;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::vector<std::string> cells = splitCsvLine(line);
            if (cells.empty()) continue;
            cad::core::Configuration config;
            config.name = cells[0];
            for (size_t i = 1; i < cells.size() && i < headers.size(); ++i) {
                try {
                    double v = std::stod(cells[i]);
                    config.parameter_overrides[headers[i]] = v;
                } catch (...) {}
            }
            part.addConfiguration(config);
        }
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool ProjectFileService::savePartConfigurationsToCsv(const cad::core::Part& part, const std::string& csv_path) const {
    try {
        if (csv_path.empty()) return false;
        const auto& configs = part.configurations();
        if (configs.empty()) return false;
        std::set<std::string> param_names;
        for (const auto& c : configs) {
            for (const auto& kv : c.parameter_overrides) param_names.insert(kv.first);
        }
        std::vector<std::string> headers = {"Name"};
        for (const auto& p : param_names) headers.push_back(p);
        std::ofstream file(csv_path);
        if (!file.is_open()) return false;
        for (size_t i = 0; i < headers.size(); ++i) {
            file << headers[i];
            if (i + 1 < headers.size()) file << ",";
        }
        file << "\n";
        for (const auto& c : configs) {
            file << c.name;
            for (size_t i = 1; i < headers.size(); ++i) {
                file << ",";
                auto it = c.parameter_overrides.find(headers[i]);
                if (it != c.parameter_overrides.end()) file << it->second;
            }
            file << "\n";
        }
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool ProjectFileService::loadProjectConfig(const std::string& config_path, ProjectConfig& out) const {
    try {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            return false;
        }
        out.working_directory.clear();
        out.search_paths.clear();
        out.library_paths.clear();
        out.template_directory.clear();
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("WORKING_DIR=") == 0) {
                out.working_directory = line.substr(12);
            } else if (line.find("SEARCH_PATH=") == 0) {
                out.search_paths.push_back(line.substr(12));
            } else if (line.find("LIBRARY_PATH=") == 0) {
                out.library_paths.push_back(line.substr(13));
            } else if (line.find("TEMPLATE_DIR=") == 0) {
                out.template_directory = line.substr(13);
            }
        }
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool ProjectFileService::saveProjectConfig(const std::string& config_path, const ProjectConfig& config) const {
    try {
        std::ofstream file(config_path);
        if (!file.is_open()) {
            return false;
        }
        file << "HYDRACAD_PROJECT_CONFIG\n";
        file << "VERSION:1\n";
        if (!config.working_directory.empty()) {
            file << "WORKING_DIR=" << config.working_directory << "\n";
        }
        for (const auto& p : config.search_paths) {
            file << "SEARCH_PATH=" << p << "\n";
        }
        for (const auto& p : config.library_paths) {
            file << "LIBRARY_PATH=" << p << "\n";
        }
        if (!config.template_directory.empty()) {
            file << "TEMPLATE_DIR=" << config.template_directory << "\n";
        }
        file.close();
        return true;
    } catch (...) {
        return false;
    }
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
        oss << "FSUPPRESSED:" << (f.suppressed ? 1 : 0) << "\n";
        oss << "FSKETCH_ID:" << f.sketch_id << "\n";
        oss << "FDEPTH:" << f.depth << "\n";
        oss << "FSYMM:" << (f.symmetric ? 1 : 0) << "\n";
        oss << "FEXTRUDE_MODE:" << static_cast<int>(f.extrude_mode) << "\n";
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
        oss << "FCIRCULAR_COUNT:" << f.circular_count << "\n";
        oss << "FCIRCULAR_ANGLE:" << f.circular_angle << "\n";
        oss << "FCIRCULAR_AXIS:" << f.circular_axis << "\n";
        oss << "FTHIN_WALL:" << (f.thin_wall ? 1 : 0) << "\n";
        oss << "FTHIN_THICKNESS:" << f.thin_thickness << "\n";
        oss << "FPATH_COUNT:" << f.path_count << "\n";
        oss << "FPATH_EQUAL:" << (f.path_equal_spacing ? 1 : 0) << "\n";
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
    oss << "PART_WORK_PLANES:" << part.workPlanes().size() << "\n";
    for (const auto& wp : part.workPlanes()) {
        oss << "WP:" << wp.id << "|" << wp.name << "|"
            << wp.origin.x << "," << wp.origin.y << "," << wp.origin.z << "|"
            << wp.normal.x << "," << wp.normal.y << "," << wp.normal.z << "|"
            << wp.base_plane << "|" << wp.offset << "\n";
    }
    oss << "PART_WORK_AXES:" << part.workAxes().size() << "\n";
    for (const auto& wa : part.workAxes()) {
        oss << "WA:" << wa.id << "|" << wa.name << "|"
            << wa.point.x << "," << wa.point.y << "," << wa.point.z << "|"
            << wa.direction.x << "," << wa.direction.y << "," << wa.direction.z << "|"
            << wa.base_axis << "\n";
    }
    oss << "PART_WORK_POINTS:" << part.workPoints().size() << "\n";
    for (const auto& wpt : part.workPoints()) {
        oss << "WPT:" << wpt.id << "|" << wpt.name << "|"
            << wpt.point.x << "," << wpt.point.y << "," << wpt.point.z << "\n";
    }
    oss << "PART_COORD_SYSTEMS:" << part.coordinateSystems().size() << "\n";
    for (const auto& cs : part.coordinateSystems()) {
        oss << "CS:" << cs.id << "|" << cs.name << "|"
            << cs.origin.x << "," << cs.origin.y << "," << cs.origin.z << "|"
            << cs.direction_x.x << "," << cs.direction_x.y << "," << cs.direction_x.z << "|"
            << cs.direction_y.x << "," << cs.direction_y.y << "," << cs.direction_y.z << "\n";
    }
    oss << "PART_PARAMETERS:" << part.userParameters().size() << "\n";
    for (const auto& p : part.userParameters()) {
        oss << "UP:" << p.name << "|" << p.value << "|" << p.expression << "\n";
    }
    oss << "PART_RULES:" << part.rules().size() << "\n";
    for (const auto& r : part.rules()) {
        oss << "R:" << r.name << "|" << r.trigger << "|" << r.condition_expression << "|" << r.then_parameter << "|" << r.then_value_expression << "\n";
    }
    oss << "PART_SKELETON_ID:" << part.skeletonPartId() << "\n";
    oss << "PART_CONFIGURATIONS:" << part.configurations().size() << "\n";
    for (const auto& c : part.configurations()) {
        oss << "CONFIG:" << c.name;
        for (const auto& kv : c.parameter_overrides) oss << "|" << kv.first << "=" << kv.second;
        oss << "\n";
    }
    oss << "PART_ACTIVE_CONFIG:" << part.activeConfigurationIndex() << "\n";
    int rp = part.rollbackPosition();
    if (rp >= 0) oss << "PART_ROLLBACK:" << rp << "\n";
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
            else if (line.find("FSUPPRESSED:") == 0) current.suppressed = (line.substr(12) == "1");
            else if (line.find("FSKETCH_ID:") == 0) current.sketch_id = line.substr(11);
            else if (line.find("FDEPTH:") == 0) current.depth = std::stod(line.substr(7));
            else if (line.find("FSYMM:") == 0) current.symmetric = (line.substr(6) == "1");
            else if (line.find("FEXTRUDE_MODE:") == 0) current.extrude_mode = static_cast<cad::core::ExtrudeMode>(std::atoi(line.substr(14).c_str()));
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
            else if (line.find("FCIRCULAR_COUNT:") == 0) current.circular_count = std::atoi(line.substr(16).c_str());
            else if (line.find("FCIRCULAR_ANGLE:") == 0) current.circular_angle = std::stod(line.substr(16));
            else if (line.find("FCIRCULAR_AXIS:") == 0) current.circular_axis = line.substr(14);
            else if (line.find("FTHIN_WALL:") == 0) current.thin_wall = (line.substr(11) == "1");
            else if (line.find("FTHIN_THICKNESS:") == 0) current.thin_thickness = std::stod(line.substr(16));
            else if (line.find("FPATH_COUNT:") == 0) current.path_count = std::atoi(line.substr(12).c_str());
            else if (line.find("FPATH_EQUAL:") == 0) current.path_equal_spacing = (line.substr(12) == "1");
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
        } else if (line.find("WP:") == 0) {
            std::string rest = line.substr(3);
            auto next = [&rest](size_t& p, char sep) -> std::string {
                size_t q = rest.find(sep, p);
                std::string s = (q == std::string::npos) ? rest.substr(p) : rest.substr(p, q - p);
                p = (q == std::string::npos) ? rest.size() : q + 1;
                return s;
            };
            size_t p = 0;
            next(p, '|');
            std::string name = next(p, '|');
            std::string o = next(p, '|');
            std::string n = next(p, '|');
            std::string base_plane = next(p, '|');
            std::string off_s = next(p, '|');
            std::string plane_type_s = next(p, '|');
            cad::core::Point3D origin{};
            cad::core::Vector3D normal{};
            if (o.size() >= 5) { size_t c1 = o.find(','), c2 = o.rfind(','); if (c1 != std::string::npos && c2 > c1) { origin.x = std::stod(o.substr(0, c1)); origin.y = std::stod(o.substr(c1 + 1, c2 - c1 - 1)); origin.z = std::stod(o.substr(c2 + 1)); } }
            if (n.size() >= 5) { size_t c1 = n.find(','), c2 = n.rfind(','); if (c1 != std::string::npos && c2 > c1) { normal.x = std::stod(n.substr(0, c1)); normal.y = std::stod(n.substr(c1 + 1, c2 - c1 - 1)); normal.z = std::stod(n.substr(c2 + 1)); } }
            double offset_val = off_s.empty() ? 0.0 : std::stod(off_s);
            std::string wp_id;
            if (!base_plane.empty()) {
                wp_id = part.addWorkPlaneOffset(name, base_plane, offset_val);
            } else {
                wp_id = part.addWorkPlane(name, origin, normal);
            }
            if (!plane_type_s.empty() && !wp_id.empty()) {
                cad::core::WorkPlane* wp_ptr = part.findWorkPlane(wp_id);
                if (wp_ptr) wp_ptr->plane_type = plane_type_s;
            }
        } else if (line.find("WA:") == 0) {
            std::string rest = line.substr(3);
            size_t p = 0;
            auto next = [&rest](size_t& pos, char sep) -> std::string {
                size_t q = rest.find(sep, pos);
                std::string s = (q == std::string::npos) ? rest.substr(pos) : rest.substr(pos, q - pos);
                pos = (q == std::string::npos) ? rest.size() : q + 1;
                return s;
            };
            next(p, '|');
            std::string name = next(p, '|');
            std::string pt_s = next(p, '|');
            std::string dir_s = next(p, '|');
            std::string base_axis = next(p, '|');
            cad::core::Point3D point{};
            cad::core::Vector3D direction{};
            if (pt_s.size() >= 5) { size_t c1 = pt_s.find(','), c2 = pt_s.rfind(','); if (c1 != std::string::npos && c2 > c1) { point.x = std::stod(pt_s.substr(0, c1)); point.y = std::stod(pt_s.substr(c1 + 1, c2 - c1 - 1)); point.z = std::stod(pt_s.substr(c2 + 1)); } }
            if (dir_s.size() >= 5) { size_t c1 = dir_s.find(','), c2 = dir_s.rfind(','); if (c1 != std::string::npos && c2 > c1) { direction.x = std::stod(dir_s.substr(0, c1)); direction.y = std::stod(dir_s.substr(c1 + 1, c2 - c1 - 1)); direction.z = std::stod(dir_s.substr(c2 + 1)); } }
            if (!base_axis.empty()) {
                part.addWorkAxisBase(name, base_axis);
            } else {
                part.addWorkAxis(name, point, direction);
            }
        } else if (line.find("WPT:") == 0) {
            std::string rest = line.substr(4);
            size_t p = 0;
            auto next = [&rest](size_t& pos, char sep) -> std::string {
                size_t q = rest.find(sep, pos);
                std::string s = (q == std::string::npos) ? rest.substr(pos) : rest.substr(pos, q - pos);
                pos = (q == std::string::npos) ? rest.size() : q + 1;
                return s;
            };
            next(p, '|');
            std::string name = next(p, '|');
            std::string pt_s = next(p, '|');
            cad::core::Point3D point{};
            if (pt_s.size() >= 5) { size_t c1 = pt_s.find(','), c2 = pt_s.rfind(','); if (c1 != std::string::npos && c2 > c1) { point.x = std::stod(pt_s.substr(0, c1)); point.y = std::stod(pt_s.substr(c1 + 1, c2 - c1 - 1)); point.z = std::stod(pt_s.substr(c2 + 1)); } }
            part.addWorkPoint(name, point);
        } else if (line.find("CS:") == 0) {
            std::string rest = line.substr(3);
            size_t p = 0;
            auto next = [&rest](size_t& pos, char sep) -> std::string {
                size_t q = rest.find(sep, pos);
                std::string s = (q == std::string::npos) ? rest.substr(pos) : rest.substr(pos, q - pos);
                pos = (q == std::string::npos) ? rest.size() : q + 1;
                return s;
            };
            next(p, '|');
            std::string name = next(p, '|');
            std::string o = next(p, '|');
            std::string dx_s = next(p, '|');
            std::string dy_s = next(p, '|');
            cad::core::Point3D origin{};
            cad::core::Vector3D dir_x{1, 0, 0}, dir_y{0, 1, 0};
            if (o.size() >= 5) { size_t c1 = o.find(','), c2 = o.rfind(','); if (c1 != std::string::npos && c2 > c1) { origin.x = std::stod(o.substr(0, c1)); origin.y = std::stod(o.substr(c1 + 1, c2 - c1 - 1)); origin.z = std::stod(o.substr(c2 + 1)); } }
            if (dx_s.size() >= 5) { size_t c1 = dx_s.find(','), c2 = dx_s.rfind(','); if (c1 != std::string::npos && c2 > c1) { dir_x.x = std::stod(dx_s.substr(0, c1)); dir_x.y = std::stod(dx_s.substr(c1 + 1, c2 - c1 - 1)); dir_x.z = std::stod(dx_s.substr(c2 + 1)); } }
            if (dy_s.size() >= 5) { size_t c1 = dy_s.find(','), c2 = dy_s.rfind(','); if (c1 != std::string::npos && c2 > c1) { dir_y.x = std::stod(dy_s.substr(0, c1)); dir_y.y = std::stod(dy_s.substr(c1 + 1, c2 - c1 - 1)); dir_y.z = std::stod(dy_s.substr(c2 + 1)); } }
            part.addCoordinateSystem(name, origin, dir_x, dir_y);
        } else if (line.find("UP:") == 0) {
            cad::core::Parameter param;
            std::string rest = line.substr(3);
            size_t pos = 0;
            size_t q = rest.find('|');
            if (q == std::string::npos) continue;
            param.name = rest.substr(pos, q - pos);
            pos = q + 1;
            q = rest.find('|', pos);
            if (q != std::string::npos && pos < rest.size()) {
                try { param.value = std::stod(rest.substr(pos, q - pos)); } catch (...) { param.value = 0.0; }
            } else {
                param.value = 0.0;
            }
            pos = (q != std::string::npos) ? q + 1 : rest.size();
            param.expression = (pos < rest.size()) ? rest.substr(pos) : "";
            part.addUserParameter(param);
        } else if (line.find("R:") == 0) {
            cad::core::Rule rule;
            std::string rest = line.substr(2);
            size_t pos = 0;
            auto next = [&rest, &pos]() -> std::string {
                size_t q = rest.find('|', pos);
                std::string s = (q == std::string::npos) ? rest.substr(pos) : rest.substr(pos, q - pos);
                pos = (q == std::string::npos) ? rest.size() : q + 1;
                return s;
            };
            rule.name = next();
            rule.trigger = next();
            rule.condition_expression = next();
            rule.then_parameter = next();
            rule.then_value_expression = next();
            part.addRule(rule);
        } else if (line.find("PART_SKELETON_ID:") == 0) {
            part.setSkeletonPartId(line.substr(17));
        } else if (line.find("CONFIG:") == 0) {
            cad::core::Configuration config;
            std::string rest = line.substr(7);
            size_t pos = 0;
            size_t q = rest.find('|');
            config.name = (q == std::string::npos) ? rest : rest.substr(0, q);
            pos = (q == std::string::npos) ? rest.size() : q + 1;
            while (pos < rest.size()) {
                q = rest.find('|', pos);
                std::string seg = (q == std::string::npos) ? rest.substr(pos) : rest.substr(pos, q - pos);
                pos = (q == std::string::npos) ? rest.size() : q + 1;
                size_t eq = seg.find('=');
                if (eq != std::string::npos) {
                    try { config.parameter_overrides[seg.substr(0, eq)] = std::stod(seg.substr(eq + 1)); } catch (...) {}
                }
            }
            part.addConfiguration(config);
        } else if (line.find("PART_ACTIVE_CONFIG:") == 0) {
            part.setActiveConfiguration(std::atoi(line.substr(19).c_str()));
        } else if (line.find("PART_ROLLBACK:") == 0) {
            part.setRollbackPosition(std::atoi(line.substr(14).c_str()));
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
            << g.width << "|" << g.height;
        if (g.type == cad::core::GeometryType::Text) {
            oss << "|" << g.text_content;
        }
        oss << "\n";
    }
    oss << "CONSTRAINTS:" << sketch.constraints().size() << "\n";
    for (const auto& c : sketch.constraints()) {
        oss << "C:" << static_cast<int>(c.type) << "|" << c.a << "|" << c.b << "|" << c.value << "\n";
    }
    oss << "PARAMETERS:" << sketch.parameters().size() << "\n";
    for (const auto& p : sketch.parameters()) {
        oss << "P:" << p.name << "|" << p.value << "|" << p.expression << "\n";
    }
    oss << "SKETCH_3D:" << (sketch.is3D() ? 1 : 0) << "\n";
    const auto& wps = sketch.waypoints3D();
    oss << "WAYPOINTS_3D:" << wps.size() << "\n";
    for (const auto& wp : wps) {
        oss << "WP3D:" << wp.x << "," << wp.y << "," << wp.z << "\n";
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
            std::string text_content = next();
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
                case cad::core::GeometryType::Ellipse:
                    new_id = sketch.addEllipse(center_pt, radius, width);
                    break;
                case cad::core::GeometryType::Polygon:
                    new_id = sketch.addPolygon({start_pt, end_pt});
                    break;
                case cad::core::GeometryType::Spline:
                    new_id = sketch.addSpline({start_pt, end_pt});
                    break;
                case cad::core::GeometryType::Text:
                    new_id = sketch.addText(start_pt, text_content);
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
        } else if (line.find("SKETCH_3D:") == 0) {
            sketch.set3D(line.substr(10) == "1");
        } else if (line.find("WP3D:") == 0) {
            std::string rest = line.substr(5);
            size_t c1 = rest.find(','), c2 = rest.rfind(',');
            if (c1 != std::string::npos && c2 > c1) {
                double x = parseDouble(rest.substr(0, c1));
                double y = parseDouble(rest.substr(c1 + 1, c2 - c1 - 1));
                double z = parseDouble(rest.substr(c2 + 1));
                sketch.addWaypoint3D(x, y, z);
            }
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
    
    oss << "JOINTS:" << assembly.joints().size() << "\n";
    for (const auto& j : assembly.joints()) {
        oss << "JOINT:" << j.component_a << "," << j.component_b << ","
            << static_cast<int>(j.type) << ","
            << j.axis_direction.x << "," << j.axis_direction.y << "," << j.axis_direction.z << ","
            << j.axis_origin.x << "," << j.axis_origin.y << "," << j.axis_origin.z << ","
            << j.slot_direction.x << "," << j.slot_direction.y << "," << j.slot_direction.z << ","
            << j.limit_low << "," << j.limit_high << "\n";
    }
    
    if (assembly.getExplosionFactor() > 0 || assembly.hasExplosionOffsets()) {
        oss << "EXPLOSION_FACTOR:" << assembly.getExplosionFactor() << "\n";
        for (std::size_t i = 0; i < assembly.components().size(); ++i) {
            cad::core::Vector3D off = assembly.getExplosionOffset(assembly.components()[i].id);
            if (off.x != 0 || off.y != 0 || off.z != 0) {
                oss << "EXPLOSION_OFFSET:" << i << "," << off.x << "," << off.y << "," << off.z << "\n";
            }
        }
    }
    oss << "ASSEMBLY_CONFIGURATIONS:" << assembly.configurations().size() << "\n";
    for (const auto& c : assembly.configurations()) {
        oss << "ASSEMBLY_CONFIG:" << c.name;
        for (const auto& kv : c.component_config_index) oss << "|" << kv.first << "=" << kv.second;
        oss << "\n";
    }
    oss << "ASSEMBLY_ACTIVE_CONFIG:" << assembly.activeConfigurationIndex() << "\n";
    for (const auto& comp : assembly.components()) {
        if (assembly.isComponentLightweight(comp.id))
            oss << "LIGHTWEIGHT:" << comp.id << "\n";
        if (assembly.isComponentFlexible(comp.id))
            oss << "FLEXIBLE:" << comp.id << "\n";
        for (const std::string& iface : assembly.getComponentInterfaces(comp.id))
            oss << "COMPONENT_INTERFACE:" << comp.id << "|" << iface << "\n";
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
            } else if (line.find("JOINTS:") == 0) {
                (void)line;  // count optional
            } else if (line.find("JOINT:") == 0) {
                std::string rest = line.substr(6);
                std::vector<double> vals;
                for (size_t pos = 0; ; ) {
                    size_t next = rest.find(',', pos);
                    if (next == std::string::npos) {
                        vals.push_back(std::stod(rest.substr(pos)));
                        break;
                    }
                    vals.push_back(std::stod(rest.substr(pos, next - pos)));
                    pos = next + 1;
                }
                if (vals.size() >= 11) {
                    cad::core::Joint j;
                    j.component_a = static_cast<std::uint64_t>(vals[0]);
                    j.component_b = static_cast<std::uint64_t>(vals[1]);
                    j.type = static_cast<cad::core::JointType>(static_cast<int>(vals[2]));
                    j.axis_direction.x = vals[3];
                    j.axis_direction.y = vals[4];
                    j.axis_direction.z = vals[5];
                    j.axis_origin.x = vals[6];
                    j.axis_origin.y = vals[7];
                    j.axis_origin.z = vals[8];
                    if (vals.size() >= 14) {
                        j.slot_direction.x = vals[9];
                        j.slot_direction.y = vals[10];
                        j.slot_direction.z = vals[11];
                        j.limit_low = vals[12];
                        j.limit_high = vals[13];
                    } else {
                        j.limit_low = vals[9];
                        j.limit_high = vals[10];
                    }
                    assembly.addJoint(j);
                }
            } else if (line.find("EXPLOSION_FACTOR:") == 0) {
                double factor = std::stod(line.substr(17));
                assembly.setExplosionFactor(factor);
            } else if (line.find("EXPLOSION_OFFSET:") == 0) {
                std::string rest = line.substr(17);
                size_t p1 = rest.find(',');
                size_t p2 = rest.find(',', p1 + 1);
                size_t p3 = rest.find(',', p2 + 1);
                if (p1 != std::string::npos && p2 != std::string::npos && p3 != std::string::npos) {
                    std::size_t idx = static_cast<std::size_t>(std::stoull(rest.substr(0, p1)));
                    double dx = std::stod(rest.substr(p1 + 1, p2 - p1 - 1));
                    double dy = std::stod(rest.substr(p2 + 1, p3 - p2 - 1));
                    double dz = std::stod(rest.substr(p3 + 1));
                    const auto& comps = assembly.components();
                    if (idx < comps.size()) {
                        assembly.setExplosionOffset(comps[idx].id, dx, dy, dz);
                    }
                }
            } else if (line.find("ASSEMBLY_CONFIG:") == 0) {
                cad::core::AssemblyConfiguration ac;
                std::string rest = line.substr(16);
                size_t pos = 0;
                size_t q = rest.find('|');
                ac.name = (q == std::string::npos) ? rest : rest.substr(0, q);
                pos = (q == std::string::npos) ? rest.size() : q + 1;
                while (pos < rest.size()) {
                    q = rest.find('|', pos);
                    std::string seg = (q == std::string::npos) ? rest.substr(pos) : rest.substr(pos, q - pos);
                    pos = (q == std::string::npos) ? rest.size() : q + 1;
                    size_t eq = seg.find('=');
                    if (eq != std::string::npos) {
                        try {
                            std::uint64_t cid = static_cast<std::uint64_t>(std::stoull(seg.substr(0, eq)));
                            int cidx = std::atoi(seg.substr(eq + 1).c_str());
                            ac.component_config_index[cid] = cidx;
                        } catch (...) {}
                    }
                }
                assembly.addConfiguration(ac);
            } else if (line.find("ASSEMBLY_ACTIVE_CONFIG:") == 0) {
                assembly.setActiveConfiguration(std::atoi(line.substr(23).c_str()));
            } else if (line.find("LIGHTWEIGHT:") == 0) {
                try {
                    std::uint64_t cid = static_cast<std::uint64_t>(std::stoull(line.substr(12)));
                    assembly.setComponentLightweight(cid, true);
                } catch (...) {}
            } else if (line.find("FLEXIBLE:") == 0) {
                try {
                    std::uint64_t cid = static_cast<std::uint64_t>(std::stoull(line.substr(8)));
                    assembly.setComponentFlexible(cid, true);
                } catch (...) {}
            } else if (line.find("COMPONENT_INTERFACE:") == 0) {
                std::string rest = line.substr(20);
                size_t pipe = rest.find('|');
                if (pipe != std::string::npos) {
                    try {
                        std::uint64_t cid = static_cast<std::uint64_t>(std::stoull(rest.substr(0, pipe)));
                        std::string name = rest.substr(pipe + 1);
                        assembly.addComponentInterface(cid, name);
                    } catch (...) {}
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
