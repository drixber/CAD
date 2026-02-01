#include "UpdateChecker.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace cad {
namespace core {
namespace updates {
namespace {

struct Semver {
    int major{0};
    int minor{0};
    int patch{0};
};

bool parseSemverTag(const std::string& tag, Semver& out) {
    if (tag.empty()) {
        return false;
    }
    std::string trimmed = tag;
    if (trimmed[0] == 'v' || trimmed[0] == 'V') {
        trimmed = trimmed.substr(1);
    }
    // Strip pre-release suffix (e.g. -beta, -rc.1) so 3.0.16-beta compares as same base version
    std::size_t dash = trimmed.find('-');
    if (dash != std::string::npos) {
        trimmed = trimmed.substr(0, dash);
    }
    std::stringstream ss(trimmed);
    std::string part;
    if (!std::getline(ss, part, '.')) return false;
    out.major = std::atoi(part.c_str());
    if (!std::getline(ss, part, '.')) return false;
    out.minor = std::atoi(part.c_str());
    if (!std::getline(ss, part, '.')) return false;
    out.patch = std::atoi(part.c_str());
    return true;
}

bool isNewer(const Semver& latest, const Semver& current) {
    if (latest.major != current.major) return latest.major > current.major;
    if (latest.minor != current.minor) return latest.minor > current.minor;
    return latest.patch > current.patch;
}

bool runCommandCapture(const std::string& command, std::string& output) {
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (!pipe) {
        return false;
    }
    std::array<char, 256> buffer{};
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
        output.append(buffer.data());
    }
#ifdef _WIN32
    int rc = _pclose(pipe);
#else
    int rc = pclose(pipe);
#endif
    return rc == 0;
}

bool extractJsonStringValue(const std::string& json,
                            const std::string& key,
                            std::string& value) {
    const std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) {
        return false;
    }
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) {
        return false;
    }
    pos = json.find('"', pos);
    if (pos == std::string::npos) {
        return false;
    }
    size_t end = pos + 1;
    while (end < json.size()) {
        end = json.find('"', end);
        if (end == std::string::npos) {
            return false;
        }
        if (json[end - 1] != '\\') {
            break;
        }
        ++end;
    }
    if (end <= pos + 1) {
        return false;
    }
    value = json.substr(pos + 1, end - pos - 1);
    return true;
}

/** Find first asset with name matching any of the given names, return its browser_download_url. */
std::string extractAssetDownloadUrl(const std::string& json,
                                    const std::vector<std::string>& asset_names) {
    for (const auto& name : asset_names) {
        const std::string name_key = "\"name\":\"" + name + "\"";
        size_t name_pos = json.find(name_key);
        if (name_pos == std::string::npos) continue;
        const std::string url_key = "\"browser_download_url\":\"";
        size_t url_start = json.find(url_key, name_pos);
        if (url_start == std::string::npos) continue;
        url_start += url_key.size();
        size_t url_end = json.find('"', url_start);
        if (url_end == std::string::npos || url_end <= url_start) continue;
        return json.substr(url_start, url_end - url_start);
    }
    return {};
}

/** Unescape JSON string (e.g. \\n -> newline, \\" -> "). */
std::string unescapeJsonString(std::string s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            if (s[i + 1] == 'n') { out += '\n'; ++i; continue; }
            if (s[i + 1] == 'r') { out += '\r'; ++i; continue; }
            if (s[i + 1] == 't') { out += '\t'; ++i; continue; }
            if (s[i + 1] == '"') { out += '"';  ++i; continue; }
            if (s[i + 1] == '\\') { out += '\\'; ++i; continue; }
        }
        out += s[i];
    }
    return out;
}

}  // namespace

UpdateInfo parseGithubReleaseResponse(const std::string& response, const std::string& currentTag) {
    UpdateInfo info{};
    if (response.empty()) {
        info.error = "Empty response from GitHub API";
        return info;
    }
    // Detect rate limit or API errors in body
    if (response.find("rate limit") != std::string::npos ||
        response.find("API rate limit exceeded") != std::string::npos) {
        info.error = "GitHub API rate limit exceeded. Please try again later.";
        return info;
    }
    if (response.find("\"message\":\"Not Found\"") != std::string::npos) {
        info.error = "Release or repository not found.";
        return info;
    }
    std::string latest_tag;
    std::string html_url;
    if (!extractJsonStringValue(response, "tag_name", latest_tag)) {
        info.error = "Failed to parse tag_name";
        return info;
    }
    if (!extractJsonStringValue(response, "html_url", html_url)) {
        info.error = "Failed to parse html_url";
        return info;
    }
    info.latestTag = latest_tag;
    info.releaseUrl = html_url;
    std::vector<std::string> asset_names;
#if defined(_WIN32)
    asset_names = {"HydraCADSetup.exe", "app-windows.zip", "HydraCAD-macos.zip", "hydracad-linux-portable.tar.gz"};
#elif defined(__APPLE__)
    asset_names = {"HydraCAD-macos.zip", "HydraCADSetup.exe", "app-windows.zip", "hydracad-linux-portable.tar.gz"};
#else
    asset_names = {"hydracad-linux-portable.tar.gz", "HydraCAD-macos.zip", "HydraCADSetup.exe", "app-windows.zip"};
#endif
    info.assetDownloadUrl = extractAssetDownloadUrl(response, asset_names);
    std::string body_raw;
    if (extractJsonStringValue(response, "body", body_raw)) {
        info.body = unescapeJsonString(body_raw);
        if (info.body.size() > 2000) info.body.resize(2000);
    }
    Semver latest{};
    Semver current{};
    if (!parseSemverTag(latest_tag, latest) || !parseSemverTag(currentTag, current)) {
        info.error = "Invalid version format";
        return info;
    }
    info.updateAvailable = isNewer(latest, current);
    // Pre-release (e.g. v3.0.16-beta) is not considered newer than stable v3.0.16
    if (info.updateAvailable && latest_tag.find('-') != std::string::npos &&
        latest.major == current.major && latest.minor == current.minor && latest.patch == current.patch) {
        info.updateAvailable = false;
    }
    return info;
}

std::string getAssetUrlFromReleaseJson(const std::string& releaseJson, const std::string& assetName) {
    return extractAssetDownloadUrl(releaseJson, {assetName});
}

std::map<std::string, std::string> parseUpdateJsonChecksums(const std::string& updateJsonBody) {
    std::map<std::string, std::string> out;
    if (updateJsonBody.empty()) return out;
    // Minimal parse: find "assets": [ ... ] and each { "name": "X", "sha256": "Y" }
    const std::string key_assets = "\"assets\"";
    size_t assets_pos = updateJsonBody.find(key_assets);
    if (assets_pos == std::string::npos) return out;
    size_t bracket = updateJsonBody.find('[', assets_pos);
    if (bracket == std::string::npos) return out;
    size_t pos = bracket + 1;
    while (pos < updateJsonBody.size()) {
        size_t name_pos = updateJsonBody.find("\"name\"", pos);
        if (name_pos == std::string::npos || name_pos > updateJsonBody.find(']', pos)) break;
        size_t colon = updateJsonBody.find(':', name_pos);
        if (colon == std::string::npos) break;
        size_t quote1 = updateJsonBody.find('"', colon);
        if (quote1 == std::string::npos) break;
        size_t quote2 = updateJsonBody.find('"', quote1 + 1);
        if (quote2 == std::string::npos) break;
        std::string name = updateJsonBody.substr(quote1 + 1, quote2 - quote1 - 1);
        size_t sha_pos = updateJsonBody.find("\"sha256\"", quote2);
        if (sha_pos == std::string::npos || sha_pos > updateJsonBody.find('}', quote2)) {
            pos = quote2 + 1;
            continue;
        }
        size_t colon2 = updateJsonBody.find(':', sha_pos);
        if (colon2 == std::string::npos) break;
        size_t q1 = updateJsonBody.find('"', colon2);
        if (q1 == std::string::npos) break;
        size_t q2 = updateJsonBody.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        std::string sha = updateJsonBody.substr(q1 + 1, q2 - q1 - 1);
        if (!name.empty() && !sha.empty()) {
            out[name] = sha;
        }
        pos = q2 + 1;
    }
    return out;
}

bool isVersionNewerThan(const std::string& latestTag, const std::string& currentTag) {
    Semver latest{};
    Semver current{};
    if (!parseSemverTag(latestTag, latest) || !parseSemverTag(currentTag, current)) {
        return false;
    }
    if (!isNewer(latest, current)) return false;
    if (latestTag.find('-') != std::string::npos &&
        latest.major == current.major && latest.minor == current.minor && latest.patch == current.patch) {
        return false;
    }
    return true;
}

UpdateInfo checkGithubLatestRelease(const std::string& owner,
                                    const std::string& repo,
                                    const std::string& currentTag) {
    UpdateInfo info{};
    if (owner.empty() || repo.empty()) {
        info.error = "Owner/Repo not set";
        return info;
    }

    const std::string url = "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";
    const std::string command = "curl -s -L -H \"User-Agent: CAD-UpdateChecker\" \"" + url + "\"";
    std::string response;
    if (!runCommandCapture(command, response)) {
        info.error = "curl failed or is not available";
        return info;
    }
    return parseGithubReleaseResponse(response, currentTag);
}

bool openUrlInBrowser(const std::string& url) {
#ifdef _WIN32
    if (url.empty()) {
        return false;
    }
    return reinterpret_cast<intptr_t>(ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
#elif __APPLE__
    if (url.empty()) {
        return false;
    }
    std::string cmd = "open \"" + url + "\"";
    return std::system(cmd.c_str()) == 0;
#else
    if (url.empty()) {
        return false;
    }
    std::string cmd = "xdg-open \"" + url + "\"";
    return std::system(cmd.c_str()) == 0;
#endif
}

}  // namespace updates
}  // namespace core
}  // namespace cad
