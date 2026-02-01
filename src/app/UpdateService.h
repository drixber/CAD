#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <memory>

namespace cad {
namespace app {

class HttpClient;

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
    
    // Update checking (GitHub Releases as primary source)
    bool checkForUpdates();
    /** Check GitHub releases/latest; fills latest_update_info_ and update_available_. Returns true if newer version found. */
    bool checkForUpdatesFromGitHub(const std::string& owner, const std::string& repo);
    UpdateInfo getLatestUpdateInfo() const;
    bool isUpdateAvailable() const;
    std::string getLastError() const { return last_error_; }
    
    // Update downloading. If target_file_path is non-empty, download there (e.g. user writable dir);
    // otherwise save as "update_<version>.exe" in current directory.
    bool downloadUpdate(const UpdateInfo& update_info,
                       std::function<void(const UpdateProgress&)> progress_callback = nullptr,
                       const std::string& target_file_path = std::string());
    /** Verify downloaded file with latest_update_info_.checksum (if set). Returns true if no checksum or match. */
    bool verifyDownloadedUpdate(const std::string& file_path) const;
    bool installUpdate(const std::string& update_file_path);
    
    // In-place update installation (without full reinstall)
    bool installInPlaceUpdate(const std::string& update_package_path,
                             std::function<void(const UpdateProgress&)> progress_callback = nullptr);
    
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
    
    /** For Qt-based GitHub API fetch when CAD_USE_QT_NETWORK (avoids curl on Windows). */
    HttpClient& getHttpClient() const;
    
private:
    std::string current_version_;
    std::string update_server_url_;
    UpdateInfo latest_update_info_;
    bool update_available_{false};
    std::string last_error_;
    bool auto_update_enabled_{true};
    int auto_update_check_interval_days_{7};
    mutable std::unique_ptr<class HttpClient> http_client_;
    
    bool fetchUpdateInfo();
    bool verifyUpdateFile(const std::string& file_path, const std::string& checksum) const;
    bool verifyUpdateChecksum(const std::string& update_file_path, const std::string& expected_checksum) const;
    std::string calculateFileChecksum(const std::string& file_path) const;
    bool extractUpdatePackage(const std::string& file_path, const std::string& extract_path) const;
    bool applyUpdate(const std::string& extract_path) const;
    void cleanupUpdateFiles(const std::string& file_path) const;
};

}  // namespace app
}  // namespace cad

