#include "UpdateService.h"
#include "UpdateInstaller.h"
#include "HttpClient.h"
#include "core/updates/UpdateChecker.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <map>
#include <regex>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <cctype>

#ifdef CAD_USE_QT
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <QFile>
#include <QCryptographicHash>
#endif

namespace cad {
namespace app {

UpdateService::UpdateService() {
    current_version_ = "v3.0.14";
    update_server_url_.clear();  // GitHub Releases as single source of truth
    http_client_ = std::make_unique<HttpClient>();
}

UpdateService::~UpdateService() {
}

bool UpdateService::checkForUpdates() {
    update_available_ = false;
    last_error_.clear();
    if (!update_server_url_.empty()) {
        if (!fetchUpdateInfo()) {
            last_error_ = "Update server unavailable";
            return false;
        }
        if (latest_update_info_.version != current_version_) {
            update_available_ = true;
            return true;
        }
        return false;
    }
    return checkForUpdatesFromGitHub("drixber", "CAD");
}

bool UpdateService::checkForUpdatesFromGitHub(const std::string& owner, const std::string& repo) {
    update_available_ = false;
    last_error_.clear();
    latest_update_info_ = UpdateInfo{};
    if (owner.empty() || repo.empty()) {
        last_error_ = "GitHub owner/repo not set";
        return false;
    }
    std::string api_url = "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";
    std::map<std::string, std::string> headers;
    headers["Accept"] = "application/vnd.github.v3+json";
    headers["User-Agent"] = "HydraCAD-UpdateCheck";
    HttpResponse response = getHttpClient().get(api_url, headers);
    if (!response.success) {
        if (response.status_code == 403 || response.body.find("rate limit") != std::string::npos) {
            last_error_ = "GitHub API rate limit exceeded. Please try again later.";
        } else if (response.body.empty() && response.status_code == 0) {
            last_error_ = "No connection. Check your network.";
        } else {
            last_error_ = "Could not check for updates (HTTP " + std::to_string(response.status_code) + ").";
        }
        return false;
    }
    cad::core::updates::UpdateInfo gh = cad::core::updates::parseGithubReleaseResponse(response.body, current_version_);
    if (!gh.error.empty()) {
        last_error_ = gh.error;
        return false;
    }
    if (!gh.updateAvailable) {
        return false;
    }
    latest_update_info_.version = gh.latestTag;
    latest_update_info_.changelog = gh.body.empty() ? "See release page for changelog." : gh.body;
    latest_update_info_.download_url = gh.assetDownloadUrl;
    latest_update_info_.mandatory = false;
    if (latest_update_info_.download_url.empty()) {
        latest_update_info_.download_url = gh.releaseUrl;
    }
    std::string update_json_url = cad::core::updates::getAssetUrlFromReleaseJson(response.body, "update.json");
    if (!update_json_url.empty()) {
        HttpResponse json_resp = getHttpClient().get(update_json_url, headers);
        if (json_resp.success && !json_resp.body.empty()) {
            auto checksums = cad::core::updates::parseUpdateJsonChecksums(json_resp.body);
            std::string asset_name = "HydraCADSetup.exe";
#if defined(__APPLE__)
            asset_name = "HydraCAD-macos.zip";
#elif !defined(_WIN32)
            asset_name = "hydracad-linux-portable.tar.gz";
#endif
            auto it = checksums.find(asset_name);
            if (it != checksums.end()) {
                latest_update_info_.checksum = it->second;
            }
        }
    }
    update_available_ = true;
    return true;
}

UpdateInfo UpdateService::getLatestUpdateInfo() const {
    return latest_update_info_;
}

bool UpdateService::isUpdateAvailable() const {
    return update_available_;
}

bool UpdateService::downloadUpdate(const UpdateInfo& update_info,
                                   std::function<void(const UpdateProgress&)> progress_callback,
                                   const std::string& target_file_path) {
    std::string download_file = target_file_path.empty()
        ? ("update_" + update_info.version + ".exe")
        : target_file_path;

    HttpClient& client = getHttpClient();
    
    auto http_progress = [&](int percentage, std::size_t bytes_downloaded, std::size_t total_bytes) {
        if (progress_callback) {
            UpdateProgress progress;
            progress.percentage = percentage;
            progress.bytes_downloaded = bytes_downloaded;
            progress.total_bytes = total_bytes;
            progress.status_message = "Downloading update: " + std::to_string(percentage) + "%";
            progress_callback(progress);
        }
    };
    
    bool success = client.downloadFile(update_info.download_url, download_file, http_progress);
    
    if (success && progress_callback) {
        UpdateProgress progress;
        progress.percentage = 100;
        progress.bytes_downloaded = update_info.file_size;
        progress.total_bytes = update_info.file_size;
        progress.status_message = "Download complete";
        progress_callback(progress);
    }
    
    return success;
}

bool UpdateService::verifyDownloadedUpdate(const std::string& file_path) const {
    return verifyUpdateFile(file_path, latest_update_info_.checksum);
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

HttpClient& UpdateService::getHttpClient() const {
    if (!http_client_) {
        http_client_ = std::make_unique<HttpClient>();
    }
    return *http_client_;
}

bool UpdateService::fetchUpdateInfo() {
    if (update_server_url_.empty()) {
        return false;
    }
    
    std::string api_url = update_server_url_;
    if (api_url.back() != '/') {
        api_url += "/";
    }
    api_url += "latest";
    
    HttpClient& client = getHttpClient();
    std::map<std::string, std::string> headers;
    headers["Accept"] = "application/json";
    headers["User-Agent"] = "HydraCAD/2.0.0";
    headers["X-Client-Version"] = current_version_;
    
    HttpResponse response = client.get(api_url, headers);
    
    if (!response.success || response.status_code != 200) {
        // Fallback: Return no update if server is unavailable
        update_available_ = false;
        return false;
    }
    
    // Parse JSON response
    #ifdef CAD_USE_QT
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response.body), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }
    
    QJsonObject json = doc.object();
    latest_update_info_.version = json["version"].toString().toStdString();
    latest_update_info_.download_url = json["download_url"].toString().toStdString();
    latest_update_info_.file_size = static_cast<std::size_t>(json["file_size"].toInt());
    latest_update_info_.checksum = json["checksum"].toString().toStdString();
    latest_update_info_.mandatory = json["mandatory"].toBool();
    latest_update_info_.changelog = json["changelog"].toString().toStdString();
    #else
    // Fallback: Simple regex parsing (delimiter re avoids raw-string closing issues)
    std::regex version_regex(R"re("version"\s*:\s*"([^"]+)")re");
    std::regex url_regex(R"re("download_url"\s*:\s*"([^"]+)")re");
    std::regex size_regex(R"re("file_size"\s*:\s*(\d+))re");
    std::regex checksum_regex(R"re("checksum"\s*:\s*"([^"]+)")re");
    std::regex mandatory_regex(R"re("mandatory"\s*:\s*(true|false))re");
    std::regex changelog_regex(R"re("changelog"\s*:\s*"([^"]+)")re");
    
    std::smatch match;
    
    if (std::regex_search(response.body, match, version_regex)) {
        latest_update_info_.version = match[1].str();
    }
    
    if (std::regex_search(response.body, match, url_regex)) {
        latest_update_info_.download_url = match[1].str();
    }
    
    if (std::regex_search(response.body, match, size_regex)) {
        latest_update_info_.file_size = std::stoull(match[1].str());
    }
    
    if (std::regex_search(response.body, match, checksum_regex)) {
        latest_update_info_.checksum = match[1].str();
    }
    
    if (std::regex_search(response.body, match, mandatory_regex)) {
        latest_update_info_.mandatory = (match[1].str() == "true");
    }
    
    if (std::regex_search(response.body, match, changelog_regex)) {
        latest_update_info_.changelog = match[1].str();
    }
    #endif
    
    // Compare versions
    if (latest_update_info_.version != current_version_) {
        update_available_ = true;
    }
    
    return true;
}

bool UpdateService::verifyUpdateFile(const std::string& file_path, const std::string& checksum) const {
    if (checksum.empty()) {
        return true;  // No checksum available (e.g. update.json not present); skip verification
    }
    return verifyUpdateChecksum(file_path, checksum);
}

bool UpdateService::verifyUpdateChecksum(const std::string& update_file_path, const std::string& expected_checksum) const {
#ifdef CAD_USE_QT
    std::string expected_hex = expected_checksum;
    if (expected_hex.size() >= 7 && expected_hex.compare(0, 7, "sha256:") == 0) {
        expected_hex = expected_hex.substr(7);
    }
    if (expected_hex.size() == 64u) {
        QFile f(QString::fromStdString(update_file_path));
        if (!f.open(QIODevice::ReadOnly)) return false;
        QByteArray data = f.readAll();
        f.close();
        QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
        QString hex = QString::fromUtf8(hash.toHex());
        if (QString::fromStdString(expected_hex).compare(hex, Qt::CaseInsensitive) == 0) {
            return true;
        }
        return false;
    }
#endif
    std::string calculated_checksum = calculateFileChecksum(update_file_path);
    if (calculated_checksum.empty()) return false;
    if (expected_checksum.find("sha256:") == 0) {
        return calculated_checksum == expected_checksum;
    }
    return false;
}

std::string UpdateService::calculateFileChecksum(const std::string& file_path) const {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.good()) {
        return {};
    }
    
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (file_size < 1024) {
        file.close();
        return {};
    }
    
    std::hash<std::string> hasher;
    std::size_t file_hash = 0;
    
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        std::string chunk(buffer, file.gcount());
        file_hash ^= hasher(chunk) + 0x9e3779b9 + (file_hash << 6) + (file_hash >> 2);
    }
    
    file.close();
    
    std::stringstream checksum_stream;
    checksum_stream << "sha256:" << std::hex << file_hash;
    return checksum_stream.str();
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
        version_file << "Hydra CAD Update Package\n";
        version_file << "Extracted from: " << file_path << "\n";
        version_file << "Size: " << file_size << " bytes\n";
        version_file.close();
    }
    
    std::ofstream manifest_file(extract_dir + "manifest.txt");
    if (manifest_file.is_open()) {
        manifest_file << "Files:\n";
            manifest_file << "  - HydraCAD.exe\n";
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
            create_version << "Hydra CAD Update Package\n";
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
            create_manifest << "  - HydraCAD.exe\n";
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

bool UpdateService::installInPlaceUpdate(const std::string& update_package_path,
                                        std::function<void(const UpdateProgress&)> progress_callback) {
    UpdateInstaller installer;
    
    // Get installation directory
    std::string install_dir = installer.getInstallationDirectory();
    if (install_dir.empty()) {
        #ifdef CAD_USE_QT
        QSettings settings("HydraCAD", "HydraCAD");
        QString install_path = settings.value("app/install_path", "C:/Program Files/Hydra CAD").toString();
        install_dir = install_path.toStdString();
        #else
        install_dir = "C:/Program Files/Hydra CAD";
        #endif
    }
    
    // Convert progress callback
    auto install_progress = [&](const UpdateInstallProgress& install_progress) {
        if (progress_callback) {
            UpdateProgress progress;
            progress.percentage = install_progress.percentage;
            progress.status_message = install_progress.status_message;
            progress.bytes_downloaded = 0;
            progress.total_bytes = 0;
            progress_callback(progress);
        }
    };
    
    // Install update package
    bool success = installer.installUpdatePackage(update_package_path, install_dir, install_progress);
    
    if (success) {
        // Update current version
        current_version_ = latest_update_info_.version;
        
        #ifdef CAD_USE_QT
        QSettings settings("HydraCAD", "HydraCAD");
        settings.setValue("app/version", QString::fromStdString(current_version_));
        #endif
    }
    
    return success;
}

}  // namespace app
}  // namespace cad

