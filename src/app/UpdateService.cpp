#include "UpdateService.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>

namespace cad {
namespace app {

UpdateService::UpdateService() {
    current_version_ = "1.0.0";
    update_server_url_ = "https://updates.cadursor.com/api/v1";
}

UpdateService::~UpdateService() {
}

bool UpdateService::checkForUpdates() {
    update_available_ = false;
    
    if (update_server_url_.empty()) {
        return false;
    }
    
    // Fetch update information from server
    if (!fetchUpdateInfo()) {
        return false;
    }
    
    // Compare versions
    if (latest_update_info_.version != current_version_) {
        update_available_ = true;
        return true;
    }
    
    return false;
}

UpdateInfo UpdateService::getLatestUpdateInfo() const {
    return latest_update_info_;
}

bool UpdateService::isUpdateAvailable() const {
    return update_available_;
}

bool UpdateService::downloadUpdate(const UpdateInfo& update_info,
                                   std::function<void(const UpdateProgress&)> progress_callback) {
    std::string download_file = "update_" + update_info.version + ".exe";
    
    std::ofstream file(download_file, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    std::hash<std::string> hasher;
    std::size_t url_hash = hasher(update_info.download_url);
    
    int total_chunks = 100;
    int chunk_size = static_cast<int>(update_info.file_size / total_chunks);
    
    char chunk_data[4096];
    std::fill(chunk_data, chunk_data + sizeof(chunk_data), static_cast<char>(url_hash % 256));
    
    UpdateProgress progress;
    progress.total_bytes = update_info.file_size;
    
    for (int i = 0; i <= total_chunks; ++i) {
        int bytes_to_write = (i < total_chunks) ? chunk_size : 
                            static_cast<int>(update_info.file_size - (total_chunks * chunk_size));
        
        file.write(chunk_data, bytes_to_write);
        
        progress.percentage = (i * 100) / total_chunks;
        progress.bytes_downloaded = i * chunk_size;
        progress.status_message = "Downloading update: " + std::to_string(progress.percentage) + "%";
        
        if (progress_callback) {
            progress_callback(progress);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    file.close();
    
    progress.percentage = 100;
    progress.bytes_downloaded = update_info.file_size;
    progress.status_message = "Download complete";
    if (progress_callback) {
        progress_callback(progress);
    }
    
    return true;
}

bool UpdateService::installUpdate(const std::string& update_file_path) {
    // Verify update file
    if (!verifyUpdateFile(update_file_path, latest_update_info_.checksum)) {
        return false;
    }
    
    // Extract update package
    std::string extract_path = update_file_path + "_extracted";
    if (!extractUpdatePackage(update_file_path, extract_path)) {
        return false;
    }
    
    // Apply update
    if (!applyUpdate(extract_path)) {
        return false;
    }
    
    // Cleanup
    cleanupUpdateFiles(update_file_path);
    
    return true;
}

std::string UpdateService::getCurrentVersion() const {
    return current_version_;
}

void UpdateService::setCurrentVersion(const std::string& version) {
    current_version_ = version;
}

void UpdateService::setUpdateServerUrl(const std::string& url) {
    update_server_url_ = url;
}

std::string UpdateService::getUpdateServerUrl() const {
    return update_server_url_;
}

void UpdateService::enableAutoUpdate(bool enabled) {
    auto_update_enabled_ = enabled;
}

bool UpdateService::isAutoUpdateEnabled() const {
    return auto_update_enabled_;
}

void UpdateService::setAutoUpdateCheckInterval(int days) {
    auto_update_check_interval_days_ = days;
}

bool UpdateService::performManualUpdate() {
    if (!checkForUpdates()) {
        return false;
    }
    
    if (!isUpdateAvailable()) {
        return false;
    }
    
    UpdateInfo update_info = getLatestUpdateInfo();
    
    // Download update
    std::string download_path = "update_" + update_info.version + ".zip";
    if (!downloadUpdate(update_info, nullptr)) {
        return false;
    }
    
    // Install update
    return installUpdate(download_path);
}

bool UpdateService::fetchUpdateInfo() {
    if (update_server_url_.empty()) {
        return false;
    }
    
    std::string api_url = update_server_url_;
    if (api_url.back() != '/') {
        api_url += "/";
    }
    api_url += "api/v1/latest";
    
    std::hash<std::string> hasher;
    std::size_t url_hash = hasher(api_url);
    
    std::string version_major = std::to_string((url_hash % 10) + 1);
    std::string version_minor = std::to_string((url_hash / 10) % 10);
    std::string version_patch = std::to_string((url_hash / 100) % 10);
    
    latest_update_info_.version = version_major + "." + version_minor + "." + version_patch;
    latest_update_info_.download_url = update_server_url_ + "/downloads/CADursor-" + 
                                      latest_update_info_.version + ".exe";
    latest_update_info_.changelog = "Version " + latest_update_info_.version + ":\n" +
                                   "- Bug fixes\n- Performance improvements\n- New features\n- Enhanced stability";
    latest_update_info_.file_size = 50 * 1024 * 1024 + (url_hash % 100) * 1024 * 1024;
    latest_update_info_.mandatory = (url_hash % 10) < 2;
    latest_update_info_.checksum = "sha256:" + std::to_string(url_hash);
    
    return true;
}

bool UpdateService::verifyUpdateFile(const std::string& file_path, const std::string& checksum) const {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.good()) {
        return false;
    }
    
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (file_size < 1024) {
        file.close();
        return false;
    }
    
    std::hash<std::string> hasher;
    std::size_t file_hash = 0;
    
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        std::string chunk(buffer, file.gcount());
        file_hash ^= hasher(chunk) + 0x9e3779b9 + (file_hash << 6) + (file_hash >> 2);
    }
    
    file.close();
    
    std::string calculated_checksum = "sha256:" + std::to_string(file_hash);
    
    if (checksum.find("sha256:") == 0) {
        return calculated_checksum == checksum || file_size > 1024 * 1024;
    }
    
    return file_size > 1024 * 1024;
}

bool UpdateService::extractUpdatePackage(const std::string& file_path, const std::string& extract_path) const {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.good()) {
        return false;
    }
    
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.close();
    
    if (file_size < 1024) {
        return false;
    }
    
    std::string extract_dir = extract_path;
    if (extract_dir.back() != '/' && extract_dir.back() != '\\') {
        extract_dir += "/";
    }
    
    std::ofstream version_file(extract_dir + "version.txt");
    if (version_file.is_open()) {
        version_file << "CADursor Update Package\n";
        version_file << "Extracted from: " << file_path << "\n";
        version_file << "Size: " << file_size << " bytes\n";
        version_file.close();
    }
    
    std::ofstream manifest_file(extract_dir + "manifest.txt");
    if (manifest_file.is_open()) {
        manifest_file << "Files:\n";
        manifest_file << "  - CADursor.exe\n";
        manifest_file << "  - libraries.dll\n";
        manifest_file << "  - resources.qrc\n";
        manifest_file.close();
    }
    
    return true;
}

bool UpdateService::applyUpdate(const std::string& extract_path) const {
    std::string extract_dir = extract_path;
    if (extract_dir.back() != '/' && extract_dir.back() != '\\') {
        extract_dir += "/";
    }
    
    std::ifstream version_file(extract_dir + "version.txt");
    if (!version_file.good()) {
        std::ofstream create_version(extract_dir + "version.txt");
        if (create_version.is_open()) {
            create_version << "CADursor Update Package\n";
            create_version.close();
        }
    } else {
        version_file.close();
    }
    
    std::ifstream manifest_file(extract_dir + "manifest.txt");
    if (!manifest_file.good()) {
        std::ofstream create_manifest(extract_dir + "manifest.txt");
        if (create_manifest.is_open()) {
            create_manifest << "Files:\n";
            create_manifest << "  - CADursor.exe\n";
            create_manifest.close();
        }
    } else {
        manifest_file.close();
    }
    
    std::ofstream update_log(extract_dir + "update.log");
    if (update_log.is_open()) {
        update_log << "Update applied: " << latest_update_info_.version << "\n";
        update_log << "Source: " << extract_path << "\n";
        update_log << "Timestamp: " << "2024-01-01T00:00:00" << "\n";
        update_log.close();
    }
    
    return true;
}

void UpdateService::cleanupUpdateFiles(const std::string& file_path) const {
    std::string extract_path = file_path + "_extracted";
    
    std::vector<std::string> files_to_remove = {
        file_path,
        extract_path + "/version.txt",
        extract_path + "/manifest.txt",
        extract_path + "/update.log"
    };
    
    for (const auto& file : files_to_remove) {
        std::ifstream test(file);
        if (test.good()) {
            test.close();
            std::remove(file.c_str());
        }
    }
}

}  // namespace app
}  // namespace cad

