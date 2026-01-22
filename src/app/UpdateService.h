#pragma once

#include <string>
#include <vector>
#include <functional>

namespace cad {
namespace app {

struct UpdateInfo {
    std::string version;
    std::string download_url;
    std::string changelog;
    std::size_t file_size{0};
    bool mandatory{false};
    std::string checksum;
};

struct UpdateProgress {
    int percentage{0};
    std::string status_message;
    std::size_t bytes_downloaded{0};
    std::size_t total_bytes{0};
};

class UpdateService {
public:
    UpdateService();
    ~UpdateService();
    
    // Update checking
    bool checkForUpdates();
    UpdateInfo getLatestUpdateInfo() const;
    bool isUpdateAvailable() const;
    
    // Update downloading
    bool downloadUpdate(const UpdateInfo& update_info, 
                       std::function<void(const UpdateProgress&)> progress_callback = nullptr);
    bool installUpdate(const std::string& update_file_path);
    
    // Current version
    std::string getCurrentVersion() const;
    void setCurrentVersion(const std::string& version);
    
    // Update server configuration
    void setUpdateServerUrl(const std::string& url);
    std::string getUpdateServerUrl() const;
    
    // Auto-update
    void enableAutoUpdate(bool enabled);
    bool isAutoUpdateEnabled() const;
    void setAutoUpdateCheckInterval(int days);
    
    // Manual update
    bool performManualUpdate();
    
private:
    std::string current_version_;
    std::string update_server_url_;
    UpdateInfo latest_update_info_;
    bool update_available_{false};
    bool auto_update_enabled_{true};
    int auto_update_check_interval_days_{7};
    
    bool fetchUpdateInfo();
    bool verifyUpdateFile(const std::string& file_path, const std::string& checksum) const;
    bool extractUpdatePackage(const std::string& file_path, const std::string& extract_path) const;
    bool applyUpdate(const std::string& extract_path) const;
    void cleanupUpdateFiles(const std::string& file_path) const;
};

}  // namespace app
}  // namespace cad

