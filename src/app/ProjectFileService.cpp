#include "ProjectFileService.h"

#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <iterator>

namespace cad {
namespace app {

ProjectFileService::ProjectFileService() = default;

bool ProjectFileService::saveProject(const std::string& file_path, const cad::core::Assembly& assembly) const {
    try {
        std::ofstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        std::string data = serializeAssembly(assembly);
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

bool ProjectFileService::loadProject(const std::string& file_path, cad::core::Assembly& assembly) const {
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
        
        if (!assembly_data.empty()) {
            return deserializeAssembly(assembly_data, assembly);
        }
        
        return false;
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

std::string ProjectFileService::serializeAssembly(const cad::core::Assembly& assembly) const {
    std::ostringstream oss;
    
    // Serialize assembly components
    oss << "COMPONENTS:" << assembly.components().size() << "\n";
    std::vector<std::string> part_ids;
    
    for (const auto& component : assembly.components()) {
        std::string part_id = component.part.name();
        part_ids.push_back(part_id);
        oss << "COMPONENT_ID:" << component.id << "\n";
        oss << "COMPONENT:" << part_id << "\n";
        oss << "TRANSFORM:" << component.transform.tx << "," 
            << component.transform.ty << "," << component.transform.tz << "\n";
    }
    
    // Serialize mates
    oss << "MATES:" << assembly.mates().size() << "\n";
    for (const auto& mate : assembly.mates()) {
        oss << "MATE:" << mate.component_a << "," << mate.component_b << "," 
            << static_cast<int>(mate.type) << "," << mate.value << "\n";
    }
    
    // Store part IDs for later retrieval
    oss << "PART_IDS:";
    for (size_t i = 0; i < part_ids.size(); ++i) {
        if (i > 0) oss << ",";
        oss << part_ids[i];
    }
    oss << "\n";
    
    return oss.str();
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
                // Try to read transform from next line
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
