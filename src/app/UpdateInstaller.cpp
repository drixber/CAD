#include "UpdateInstaller.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <filesystem>
#include <ctime>
#include <cstdlib>

#ifdef CAD_USE_QT
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QFile>
#endif

namespace cad {
namespace app {

UpdateInstaller::UpdateInstaller() {
    #ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    QString install_path = settings.value("app/install_path", "").toString();
    if (install_path.isEmpty()) {
        // Try to get from registry or default
        install_path = "C:/Program Files/Hydra CAD";
    }
    installation_directory_ = install_path.toStdString();
    #else
    installation_directory_ = "C:/Program Files/Hydra CAD";
    #endif
}

bool UpdateInstaller::installUpdatePackage(const std::string& package_path,
                                           const std::string& install_directory,
                                           std::function<void(const UpdateInstallProgress&)> progress_callback) {
    UpdateInstallProgress progress;
    progress.status_message = "Starting update installation...";
    if (progress_callback) progress_callback(progress);
    
    // Check if we can install
    if (!canInstallUpdate(install_directory)) {
        progress.failed = true;
        progress.error_message = "Cannot install update: Insufficient permissions or files locked";
        if (progress_callback) progress_callback(progress);
        return false;
    }
    
    // Create backup
    std::string backup_path = install_directory + "/backup_" + std::to_string(std::time(nullptr));
    progress.status_message = "Creating backup...";
    progress.percentage = 5;
    if (progress_callback) progress_callback(progress);
    
    if (!createBackup(install_directory, backup_path)) {
        progress.failed = true;
        progress.error_message = "Failed to create backup";
        if (progress_callback) progress_callback(progress);
        return false;
    }
    
    // Extract package
    std::string extract_path = package_path + "_extracted";
    progress.status_message = "Extracting update package...";
    progress.percentage = 10;
    if (progress_callback) progress_callback(progress);
    
    if (!extractUpdatePackage(package_path, extract_path)) {
        progress.failed = true;
        progress.error_message = "Failed to extract update package";
        if (progress_callback) progress_callback(progress);
        return false;
    }
    
    // Parse manifest
    std::string manifest_path = extract_path + "/manifest.json";
    UpdateManifest manifest;
    progress.status_message = "Reading update manifest...";
    progress.percentage = 20;
    if (progress_callback) progress_callback(progress);
    
    if (!parseManifest(manifest_path, manifest)) {
        progress.failed = true;
        progress.error_message = "Failed to parse update manifest";
        if (progress_callback) progress_callback(progress);
        return false;
    }
    
    // Verify files
    progress.status_message = "Verifying update files...";
    progress.percentage = 30;
    if (progress_callback) progress_callback(progress);
    
    if (!verifyUpdateFiles(manifest, extract_path)) {
        progress.failed = true;
        progress.error_message = "Update file verification failed";
        if (progress_callback) progress_callback(progress);
        return false;
    }
    
    // Install files
    int total_files = static_cast<int>(manifest.files_to_update.size() + 
                                      manifest.files_to_add.size() + 
                                      manifest.files_to_remove.size());
    int current_file = 0;
    
    // Update existing files
    for (const auto& file : manifest.files_to_update) {
        current_file++;
        progress.current_file = file;
        progress.status_message = "Updating " + file + "...";
        progress.percentage = 30 + (current_file * 50 / total_files);
        if (progress_callback) progress_callback(progress);
        
        std::string source = extract_path + "/" + file;
        std::string destination = install_directory + "/" + file;
        
        // Ensure destination directory exists
        std::filesystem::path dest_path(destination);
        std::filesystem::create_directories(dest_path.parent_path());
        
        if (!replaceFile(source, destination)) {
            progress.failed = true;
            progress.error_message = "Failed to update file: " + file;
            if (progress_callback) progress_callback(progress);
            
            // Rollback on failure
            rollback(backup_path, install_directory);
            return false;
        }
    }
    
    // Add new files
    for (const auto& file : manifest.files_to_add) {
        current_file++;
        progress.current_file = file;
        progress.status_message = "Adding " + file + "...";
        progress.percentage = 30 + (current_file * 50 / total_files);
        if (progress_callback) progress_callback(progress);
        
        std::string source = extract_path + "/" + file;
        std::string destination = install_directory + "/" + file;
        
        std::filesystem::path dest_path(destination);
        std::filesystem::create_directories(dest_path.parent_path());
        
        if (!replaceFile(source, destination)) {
            progress.failed = true;
            progress.error_message = "Failed to add file: " + file;
            if (progress_callback) progress_callback(progress);
            
            rollback(backup_path, install_directory);
            return false;
        }
    }
    
    // Remove obsolete files
    for (const auto& file : manifest.files_to_remove) {
        current_file++;
        progress.current_file = file;
        progress.status_message = "Removing " + file + "...";
        progress.percentage = 30 + (current_file * 50 / total_files);
        if (progress_callback) progress_callback(progress);
        
        std::string file_path = install_directory + "/" + file;
        if (std::filesystem::exists(file_path)) {
            deleteFile(file_path);
        }
    }
    
    // Run install script if present
    if (!manifest.install_script.empty()) {
        progress.status_message = "Running install script...";
        progress.percentage = 90;
        if (progress_callback) progress_callback(progress);
        
        std::string script_path = extract_path + "/" + manifest.install_script;
        if (std::filesystem::exists(script_path)) {
            // Execute script (platform-specific)
            #ifdef _WIN32
            std::string command = "\"" + script_path + "\"";
            int result = std::system(command.c_str());
            if (result != 0) {
                progress.failed = true;
                progress.error_message = "Install script failed";
                if (progress_callback) progress_callback(progress);
                rollback(backup_path, install_directory);
                return false;
            }
            #endif
        }
    }
    
    // Cleanup
    progress.status_message = "Cleaning up...";
    progress.percentage = 95;
    if (progress_callback) progress_callback(progress);
    
    // Cleanup extracted files (but keep backup)
    try {
        std::filesystem::remove_all(extract_path);
    } catch (...) {
        // Ignore cleanup errors
    }
    
    // Update version info
    progress.status_message = "Update completed successfully!";
    progress.percentage = 100;
    progress.completed = true;
    if (progress_callback) progress_callback(progress);
    
    return true;
}

bool UpdateInstaller::installDeltaUpdate(const UpdateManifest& manifest,
                                         const std::string& update_files_directory,
                                         const std::string& install_directory,
                                         std::function<void(const UpdateInstallProgress&)> progress_callback) {
    UpdateInstallProgress progress;
    progress.status_message = "Starting delta update...";
    if (progress_callback) progress_callback(progress);
    
    // Create backup
    std::string backup_path = install_directory + "/backup_" + std::to_string(std::time(nullptr));
    progress.status_message = "Creating backup...";
    progress.percentage = 5;
    if (progress_callback) progress_callback(progress);
    
    if (!createBackup(install_directory, backup_path)) {
        progress.failed = true;
        progress.error_message = "Failed to create backup";
        if (progress_callback) progress_callback(progress);
        return false;
    }
    
    // Verify files
    progress.status_message = "Verifying update files...";
    progress.percentage = 20;
    if (progress_callback) progress_callback(progress);
    
    if (!verifyUpdateFiles(manifest, update_files_directory)) {
        progress.failed = true;
        progress.error_message = "Update file verification failed";
        if (progress_callback) progress_callback(progress);
        return false;
    }
    
    // Install files
    int total_files = static_cast<int>(manifest.files_to_update.size() + 
                                      manifest.files_to_add.size() + 
                                      manifest.files_to_remove.size());
    int current_file = 0;
    
    // Update existing files
    for (const auto& file : manifest.files_to_update) {
        current_file++;
        progress.current_file = file;
        progress.status_message = "Updating " + file + "...";
        progress.percentage = 30 + (current_file * 60 / total_files);
        if (progress_callback) progress_callback(progress);
        
        std::string source = update_files_directory + "/" + file;
        std::string destination = install_directory + "/" + file;
        
        // Wait for file to be unlocked if needed
        if (isFileLocked(destination)) {
            if (!waitForFileUnlock(destination)) {
                progress.failed = true;
                progress.error_message = "File is locked: " + file;
                if (progress_callback) progress_callback(progress);
                rollback(backup_path, install_directory);
                return false;
            }
        }
        
        std::filesystem::path dest_path(destination);
        std::filesystem::create_directories(dest_path.parent_path());
        
        if (!replaceFile(source, destination)) {
            progress.failed = true;
            progress.error_message = "Failed to update file: " + file;
            if (progress_callback) progress_callback(progress);
            rollback(backup_path, install_directory);
            return false;
        }
    }
    
    // Add new files
    for (const auto& file : manifest.files_to_add) {
        current_file++;
        progress.current_file = file;
        progress.status_message = "Adding " + file + "...";
        progress.percentage = 30 + (current_file * 60 / total_files);
        if (progress_callback) progress_callback(progress);
        
        std::string source = update_files_directory + "/" + file;
        std::string destination = install_directory + "/" + file;
        
        std::filesystem::path dest_path(destination);
        std::filesystem::create_directories(dest_path.parent_path());
        
        if (!replaceFile(source, destination)) {
            progress.failed = true;
            progress.error_message = "Failed to add file: " + file;
            if (progress_callback) progress_callback(progress);
            rollback(backup_path, install_directory);
            return false;
        }
    }
    
    // Remove obsolete files
    for (const auto& file : manifest.files_to_remove) {
        current_file++;
        progress.current_file = file;
        progress.status_message = "Removing " + file + "...";
        progress.percentage = 30 + (current_file * 60 / total_files);
        if (progress_callback) progress_callback(progress);
        
        std::string file_path = install_directory + "/" + file;
        if (std::filesystem::exists(file_path)) {
            deleteFile(file_path);
        }
    }
    
    progress.status_message = "Delta update completed successfully!";
    progress.percentage = 100;
    progress.completed = true;
    if (progress_callback) progress_callback(progress);
    
    return true;
}

bool UpdateInstaller::createBackup(const std::string& install_directory, const std::string& backup_path) {
    try {
        if (!std::filesystem::exists(install_directory)) {
            return false;
        }
        
        // Create backup directory
        std::filesystem::create_directories(backup_path);
        
        // Copy all files to backup
        for (const auto& entry : std::filesystem::recursive_directory_iterator(install_directory)) {
            if (entry.is_regular_file()) {
                std::filesystem::path relative_path = std::filesystem::relative(entry.path(), install_directory);
                std::filesystem::path backup_file = std::filesystem::path(backup_path) / relative_path;
                
                std::filesystem::create_directories(backup_file.parent_path());
                std::filesystem::copy_file(entry.path(), backup_file, 
                                         std::filesystem::copy_options::overwrite_existing);
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool UpdateInstaller::rollback(const std::string& backup_path, const std::string& install_directory) {
    try {
        if (!std::filesystem::exists(backup_path)) {
            return false;
        }
        
        // Restore all files from backup
        for (const auto& entry : std::filesystem::recursive_directory_iterator(backup_path)) {
            if (entry.is_regular_file()) {
                std::filesystem::path relative_path = std::filesystem::relative(entry.path(), backup_path);
                std::filesystem::path restore_file = std::filesystem::path(install_directory) / relative_path;
                
                std::filesystem::create_directories(restore_file.parent_path());
                std::filesystem::copy_file(entry.path(), restore_file, 
                                         std::filesystem::copy_options::overwrite_existing);
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool UpdateInstaller::verifyUpdateFiles(const UpdateManifest& manifest, const std::string& update_files_directory) {
    for (const auto& file : manifest.files_to_update) {
        std::string file_path = update_files_directory + "/" + file;
        if (!std::filesystem::exists(file_path)) {
            return false;
        }
        
        if (manifest.file_checksums.find(file) != manifest.file_checksums.end()) {
            std::string expected_checksum = manifest.file_checksums.at(file);
            if (!verifyFileChecksum(file_path, expected_checksum)) {
                return false;
            }
        }
    }
    
    for (const auto& file : manifest.files_to_add) {
        std::string file_path = update_files_directory + "/" + file;
        if (!std::filesystem::exists(file_path)) {
            return false;
        }
    }
    
    return true;
}

std::string UpdateInstaller::getInstallationDirectory() const {
    return installation_directory_;
}

bool UpdateInstaller::canInstallUpdate(const std::string& install_directory) const {
    try {
        if (!std::filesystem::exists(install_directory)) {
            return false;
        }
        
        // Check if we can write to the directory
        std::filesystem::path test_file = std::filesystem::path(install_directory) / "update_test.tmp";
        std::ofstream test(test_file.string());
        if (!test.is_open()) {
            return false;
        }
        test.close();
        std::filesystem::remove(test_file);
        
        return true;
    } catch (...) {
        return false;
    }
}

bool UpdateInstaller::extractUpdatePackage(const std::string& package_path, const std::string& extract_path) {
    try {
        std::filesystem::create_directories(extract_path);
        
        // Check if it's a directory (already extracted)
        if (std::filesystem::is_directory(package_path)) {
            // Copy directory contents
            for (const auto& entry : std::filesystem::recursive_directory_iterator(package_path)) {
                if (entry.is_regular_file()) {
                    std::filesystem::path relative_path = std::filesystem::relative(entry.path(), package_path);
                    std::filesystem::path dest_file = std::filesystem::path(extract_path) / relative_path;
                    std::filesystem::create_directories(dest_file.parent_path());
                    std::filesystem::copy_file(entry.path(), dest_file, 
                                             std::filesystem::copy_options::overwrite_existing);
                }
            }
            return true;
        }
        
        // Check if it's a ZIP file
        std::string lower_path = package_path;
        std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), ::tolower);
        if (lower_path.find(".zip") != std::string::npos) {
            // Use system unzip command (Windows: PowerShell, Linux/Mac: unzip)
            #ifdef _WIN32
            std::string command = "powershell -Command \"Expand-Archive -Path '" + package_path + "' -DestinationPath '" + extract_path + "' -Force\"";
            int result = std::system(command.c_str());
            return result == 0;
            #else
            std::string command = "unzip -q -o '" + package_path + "' -d '" + extract_path + "'";
            int result = std::system(command.c_str());
            return result == 0;
            #endif
        }
        
        // If it's a regular file, try to copy it
        if (std::filesystem::is_regular_file(package_path)) {
            std::filesystem::path dest_file = std::filesystem::path(extract_path) / std::filesystem::path(package_path).filename();
            std::filesystem::copy_file(package_path, dest_file, 
                                     std::filesystem::copy_options::overwrite_existing);
            return true;
        }
        
        return false;
    } catch (...) {
        return false;
    }
}

bool UpdateInstaller::parseManifest(const std::string& manifest_path, UpdateManifest& manifest) {
    try {
        std::ifstream file(manifest_path);
        if (!file.is_open()) {
            return false;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Simple manifest format: "file:checksum" or "file"
            if (line.find("version:") == 0) {
                manifest.version = line.substr(7);
            } else if (line.find("update:") == 0) {
                std::string file_info = line.substr(7);
                size_t colon_pos = file_info.find(':');
                if (colon_pos != std::string::npos) {
                    std::string file = file_info.substr(0, colon_pos);
                    std::string checksum = file_info.substr(colon_pos + 1);
                    manifest.files_to_update.push_back(file);
                    manifest.file_checksums[file] = checksum;
                } else {
                    manifest.files_to_update.push_back(file_info);
                }
            } else if (line.find("add:") == 0) {
                manifest.files_to_add.push_back(line.substr(4));
            } else if (line.find("remove:") == 0) {
                manifest.files_to_remove.push_back(line.substr(7));
            } else if (line.find("script:") == 0) {
                manifest.install_script = line.substr(7);
            }
        }
        
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

bool UpdateInstaller::replaceFile(const std::string& source, const std::string& destination) {
    try {
        // Ensure destination directory exists
        std::filesystem::path dest_path(destination);
        std::filesystem::create_directories(dest_path.parent_path());
        
        // Copy file
        std::filesystem::copy_file(source, destination, 
                                 std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        return false;
    }
}

bool UpdateInstaller::deleteFile(const std::string& file_path) {
    try {
        if (std::filesystem::exists(file_path)) {
            std::filesystem::remove(file_path);
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}

bool UpdateInstaller::verifyFileChecksum(const std::string& file_path, const std::string& expected_checksum) {
    std::string calculated = calculateFileChecksum(file_path);
    if (calculated.empty()) {
        return false;
    }
    
    if (expected_checksum.find("sha256:") == 0) {
        return calculated == expected_checksum;
    }
    
    return calculated == expected_checksum;
}

std::string UpdateInstaller::calculateFileChecksum(const std::string& file_path) const {
    #ifdef CAD_USE_QT
    QFile file(QString::fromStdString(file_path));
    if (!file.open(QFile::ReadOnly)) {
        return "";
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        return "sha256:" + hash.result().toHex().toStdString();
    }
    return "";
    #else
    // Fallback for non-Qt builds
    std::ifstream file(file_path, std::ios::binary);
    if (!file.good()) {
        return "";
    }
    
    std::hash<std::string> hasher;
    std::size_t file_hash = 0;
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        std::string chunk(buffer, file.gcount());
        file_hash ^= hasher(chunk) + 0x9e3779b9 + (file_hash << 6) + (file_hash >> 2);
    }
    
    std::stringstream checksum_stream;
    checksum_stream << "sha256:" << std::hex << file_hash;
    return checksum_stream.str();
    #endif
}

bool UpdateInstaller::isFileLocked(const std::string& file_path) const {
    try {
        std::ifstream file(file_path, std::ios::app);
        if (!file.is_open()) {
            return true; // File might be locked
        }
        file.close();
        return false;
    } catch (...) {
        return true;
    }
}

bool UpdateInstaller::waitForFileUnlock(const std::string& file_path, int max_wait_seconds) const {
    for (int i = 0; i < max_wait_seconds * 10; ++i) {
        if (!isFileLocked(file_path)) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return false;
}

}  // namespace app
}  // namespace cad
