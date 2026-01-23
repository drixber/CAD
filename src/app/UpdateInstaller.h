#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <filesystem>

namespace cad {
namespace app {

struct UpdateManifest {
    std::string version;
    std::vector<std::string> files_to_update;
    std::vector<std::string> files_to_add;
    std::vector<std::string> files_to_remove;
    std::map<std::string, std::string> file_checksums;
    std::string install_script; // Optional script to run after update
};

struct UpdateInstallProgress {
    int percentage{0};
    std::string current_file;
    std::string status_message;
    bool completed{false};
    bool failed{false};
    std::string error_message;
};

class UpdateInstaller {
public:
    UpdateInstaller();
    ~UpdateInstaller() = default;
    
    // Install update package (ZIP or directory)
    bool installUpdatePackage(const std::string& package_path,
                             const std::string& install_directory,
                             std::function<void(const UpdateInstallProgress&)> progress_callback = nullptr);
    
    // Install delta update (only changed files)
    bool installDeltaUpdate(const UpdateManifest& manifest,
                           const std::string& update_files_directory,
                           const std::string& install_directory,
                           std::function<void(const UpdateInstallProgress&)> progress_callback = nullptr);
    
    // Create backup before update
    bool createBackup(const std::string& install_directory, const std::string& backup_path);
    
    // Rollback to backup
    bool rollback(const std::string& backup_path, const std::string& install_directory);
    
    // Verify update files
    bool verifyUpdateFiles(const UpdateManifest& manifest, const std::string& update_files_directory);
    
    // Get installation directory
    std::string getInstallationDirectory() const;
    
    // Check if update can be installed (file permissions, etc.)
    bool canInstallUpdate(const std::string& install_directory) const;

private:
    std::string installation_directory_;
    
    bool extractUpdatePackage(const std::string& package_path, const std::string& extract_path);
    bool parseManifest(const std::string& manifest_path, UpdateManifest& manifest);
    bool replaceFile(const std::string& source, const std::string& destination);
    bool deleteFile(const std::string& file_path);
    bool verifyFileChecksum(const std::string& file_path, const std::string& expected_checksum);
    std::string calculateFileChecksum(const std::string& file_path) const;
    bool isFileLocked(const std::string& file_path) const;
    bool waitForFileUnlock(const std::string& file_path, int max_wait_seconds = 10) const;
};

}  // namespace app
}  // namespace cad
