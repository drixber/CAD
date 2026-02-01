#include "UserAuthService.h"

#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <random>
#include <functional>
#include <cctype>
#ifdef CAD_USE_QT
#include <QSettings>
#include <QCryptographicHash>
#include <QByteArrayView>
#include <QJsonDocument>
#include <QJsonObject>
#endif
#ifdef CAD_USE_QT_NETWORK
#include <QUrlQuery>
#endif

namespace cad {
namespace app {

UserAuthService::UserAuthService() {
    #ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    QString data_dir = settings.value("app/data_directory", "data").toString();
    users_file_path_ = (data_dir + "/users.dat").toStdString();
    #else
    users_file_path_ = "data/users.dat";
    #endif
    
    // Create data directory if it doesn't exist
    std::filesystem::path data_path = std::filesystem::path(users_file_path_).parent_path();
    if (!std::filesystem::exists(data_path)) {
        std::filesystem::create_directories(data_path);
    }
}

UserAuthService::~UserAuthService() = default;

std::string UserAuthService::hashPassword(const std::string& password) const {
    #ifdef CAD_USE_QT
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(QByteArrayView(password.data(), static_cast<qsizetype>(password.length())));
    std::string salt = generateSalt();
    hash.addData(QByteArrayView(salt.data(), static_cast<qsizetype>(salt.length())));
    return salt + ":" + hash.result().toHex().toStdString();
    #else
    // Simple hash for non-Qt builds (not secure, but functional)
    std::hash<std::string> hasher;
    std::string salt = generateSalt();
    std::size_t hash_value = hasher(password + salt);
    std::ostringstream oss;
    oss << salt << ":" << std::hex << hash_value;
    return oss.str();
    #endif
}

bool UserAuthService::verifyPassword(const std::string& password, const std::string& hash) const {
    size_t colon_pos = hash.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }
    
    std::string salt = hash.substr(0, colon_pos);
    std::string stored_hash = hash.substr(colon_pos + 1);
    
    #ifdef CAD_USE_QT
    QCryptographicHash hash_calc(QCryptographicHash::Sha256);
    hash_calc.addData(QByteArrayView(password.data(), static_cast<qsizetype>(password.length())));
    hash_calc.addData(QByteArrayView(salt.data(), static_cast<qsizetype>(salt.length())));
    std::string calculated_hash = hash_calc.result().toHex().toStdString();
    return calculated_hash == stored_hash;
    #else
    std::hash<std::string> hasher;
    std::size_t hash_value = hasher(password + salt);
    std::ostringstream oss;
    oss << std::hex << hash_value;
    return oss.str() == stored_hash;
    #endif
}

std::string UserAuthService::generateSalt() const {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(chars.length() - 1));
    
    std::string salt;
    salt.reserve(16);
    for (int i = 0; i < 16; ++i) {
        salt += chars[dis(gen)];
    }
    return salt;
}

std::string UserAuthService::getTimestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::vector<User> UserAuthService::loadUsers() const {
    std::vector<User> users;
    
    try {
        if (!std::filesystem::exists(users_file_path_)) {
            return users;
        }
        
        std::ifstream file(users_file_path_, std::ios::binary);
        if (!file.is_open()) {
            return users;
        }
        
        std::string line;
        User current_user;
        int field_count = 0;
        
        while (std::getline(file, line)) {
            if (line == "USER_START") {
                current_user = User();
                field_count = 0;
            } else if (line == "USER_END") {
                users.push_back(current_user);
            } else if (line.find("USERNAME:") == 0) {
                current_user.username = line.substr(9);
            } else if (line.find("EMAIL:") == 0) {
                current_user.email = line.substr(6);
            } else if (line.find("PASSWORD_HASH:") == 0) {
                current_user.password_hash = line.substr(14);
            } else if (line.find("CREATED:") == 0) {
                current_user.created_date = line.substr(8);
            } else if (line.find("LAST_LOGIN:") == 0) {
                current_user.last_login_date = line.substr(11);
            } else if (line.find("ACTIVE:") == 0) {
                current_user.is_active = (line.substr(7) == "true");
            }
        }
        
        file.close();
    } catch (...) {
        // Return empty list on error
    }
    
    return users;
}

bool UserAuthService::saveUsers(const std::vector<User>& users) const {
    try {
        std::ofstream file(users_file_path_, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        for (const auto& user : users) {
            file << "USER_START\n";
            file << "USERNAME:" << user.username << "\n";
            file << "EMAIL:" << user.email << "\n";
            file << "PASSWORD_HASH:" << user.password_hash << "\n";
            file << "CREATED:" << user.created_date << "\n";
            file << "LAST_LOGIN:" << user.last_login_date << "\n";
            file << "ACTIVE:" << (user.is_active ? "true" : "false") << "\n";
            file << "USER_END\n";
        }
        
        file.close();
        return true;
    } catch (...) {
        return false;
    }
}

std::string UserAuthService::getApiBaseUrl() const {
    return auth_config_.getApiBaseUrl();
}

void UserAuthService::setApiBaseUrl(const std::string& url) {
    auth_config_.setApiBaseUrl(url);
}

std::string UserAuthService::getAccessToken() const {
#ifdef CAD_USE_QT
    if (!auth_config_.isApiMode() || !is_logged_in_) return {};
    if (!access_token_.empty()) return access_token_;
    QSettings settings("HydraCAD", "HydraCAD");
    return settings.value("auth/access_token").toString().toStdString();
#else
    return access_token_;
#endif
}

#ifdef CAD_USE_QT_NETWORK
static std::string extractJsonString(const std::string& body, const std::string& key) {
    std::string needle = "\"" + key + "\":\"";
    size_t pos = body.find(needle);
    if (pos == std::string::npos) return {};
    pos += needle.size();
    size_t end = pos;
    while (end < body.size() && (body[end] != '"' || (end > pos && body[end - 1] == '\\'))) ++end;
    if (end > body.size()) return {};
    return body.substr(pos, end - pos);
}
static std::string extractJsonStringOrNull(const std::string& body, const std::string& key) {
    std::string needle = "\"" + key + "\":";
    size_t pos = body.find(needle);
    if (pos == std::string::npos) return {};
    pos += needle.size();
    while (pos < body.size() && (body[pos] == ' ' || body[pos] == '\t')) ++pos;
    if (pos < body.size() && body[pos] == '"') {
        ++pos;
        size_t end = pos;
        while (end < body.size() && (body[end] != '"' || (end > pos && body[end - 1] == '\\'))) ++end;
        return body.substr(pos, end - pos);
    }
    return {};
}
#endif

void UserAuthService::storeTokens(const std::string& access, const std::string& refresh, bool remember) {
    access_token_ = access;
    refresh_token_ = refresh;
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    if (remember && !refresh.empty()) {
        settings.setValue("auth/refresh_token", QString::fromStdString(refresh));
    } else {
        settings.remove("auth/refresh_token");
    }
    if (!access.empty()) {
        settings.setValue("auth/access_token", QString::fromStdString(access));
    } else {
        settings.remove("auth/access_token");
    }
    settings.sync();
#endif
}

void UserAuthService::clearStoredTokens() {
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    settings.remove("auth/refresh_token");
    settings.remove("auth/access_token");
    settings.sync();
#endif
    access_token_.clear();
    refresh_token_.clear();
}

bool UserAuthService::fetchMeAndSetUser(const std::string& access_token) {
#ifdef CAD_USE_QT_NETWORK
    std::string base = auth_config_.getApiBaseUrl();
    if (base.empty()) return false;
    std::string url = base + "/api/auth/me";
    std::map<std::string, std::string> headers = {
        {"Authorization", "Bearer " + access_token},
        {"Content-Type", "application/json"}
    };
    cad::app::HttpResponse resp = http_client_.get(url, headers);
    if (!resp.success || resp.status_code != 200) return false;
    std::string id_s = extractJsonString(resp.body, "id");
    std::string username = extractJsonString(resp.body, "username");
    std::string email = extractJsonString(resp.body, "email");
    if (username.empty()) return false;
    current_user_.username = username;
    current_user_.email = email;
    current_user_.last_login_date = getTimestamp();
    current_user_.is_active = true;
    is_logged_in_ = true;
    return true;
#else
    (void)access_token;
    return false;
#endif
}

bool UserAuthService::refreshAndSetUser() {
#ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    QString ref = settings.value("auth/refresh_token").toString();
    if (ref.isEmpty()) return false;
    std::string refresh = ref.toStdString();
#else
    if (refresh_token_.empty()) return false;
    std::string refresh = refresh_token_;
#endif
#ifdef CAD_USE_QT_NETWORK
    std::string base = auth_config_.getApiBaseUrl();
    if (base.empty()) return false;
    std::string url = base + "/api/auth/refresh";
    std::string body = "{\"refresh_token\":\"" + refresh + "\"}";
    std::map<std::string, std::string> headers = {{"Content-Type", "application/json"}};
    cad::app::HttpResponse resp = http_client_.post(url, body, headers);
    if (!resp.success || resp.status_code != 200) return false;
    std::string access = extractJsonString(resp.body, "access_token");
    std::string new_refresh = extractJsonString(resp.body, "refresh_token");
    if (access.empty()) return false;
    storeTokens(access, new_refresh, true);
    return fetchMeAndSetUser(access);
#else
    (void)refresh;
    return false;
#endif
}

RegisterResult UserAuthService::registerViaApi(const std::string& username,
                                               const std::string& email,
                                               const std::string& password) {
    RegisterResult result;
#ifdef CAD_USE_QT_NETWORK
    std::string base = auth_config_.getApiBaseUrl();
    if (base.empty()) {
        result.error_message = "API base URL not set.";
        return result;
    }
    if (username.length() < 3) {
        result.error_message = "Username must be at least 3 characters long";
        return result;
    }
    if (email.empty() || email.find('@') == std::string::npos) {
        result.error_message = "Invalid email address";
        return result;
    }
    if (!validatePassword(password)) {
        result.error_message = "Password does not meet requirements (min 8 characters, at least one letter and one number)";
        return result;
    }
    std::string url = base + "/api/auth/register";
    std::string body = "{\"username\":\"" + username + "\",\"email\":\"" + email + "\",\"password\":\"" + password + "\"}";
    std::map<std::string, std::string> headers = {{"Content-Type", "application/json"}};
    cad::app::HttpResponse resp = http_client_.post(url, body, headers);
    if (resp.status_code == 400) {
        result.error_message = extractJsonStringOrNull(resp.body, "detail").empty() ? resp.body : extractJsonStringOrNull(resp.body, "detail");
        if (result.error_message.empty()) result.error_message = "Registration failed (e.g. username or email already taken).";
        return result;
    }
    if (!resp.success || resp.status_code != 200) {
        result.error_message = "Server error or network failure. Check API URL and try again.";
        return result;
    }
    std::string access = extractJsonString(resp.body, "access_token");
    std::string refresh = extractJsonString(resp.body, "refresh_token");
    if (access.empty()) {
        result.error_message = "Invalid server response (no token).";
        return result;
    }
    storeTokens(access, refresh, true);
    if (!fetchMeAndSetUser(access)) {
        result.error_message = "Registration succeeded but could not load user.";
        return result;
    }
    result.success = true;
    result.user = current_user_;
    return result;
#else
    result.error_message = "Backend API requires Qt Network. Unset API URL for local mode.";
    return result;
#endif
}

LoginResult UserAuthService::loginViaApi(const std::string& username,
                                        const std::string& password,
                                        bool remember_me) {
    LoginResult result;
#ifdef CAD_USE_QT_NETWORK
    std::string base = auth_config_.getApiBaseUrl();
    if (base.empty()) {
        result.error_message = "API base URL not set.";
        return result;
    }
    std::string url = base + "/api/auth/login";
    std::string body = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";
    std::map<std::string, std::string> headers = {{"Content-Type", "application/json"}};
    cad::app::HttpResponse resp = http_client_.post(url, body, headers);
    if (resp.status_code == 401) {
        result.error_message = "Invalid username or password.";
        return result;
    }
    if (!resp.success || resp.status_code != 200) {
        result.error_message = "Server error or network failure. Check API URL and try again.";
        return result;
    }
    std::string access = extractJsonString(resp.body, "access_token");
    std::string refresh = extractJsonString(resp.body, "refresh_token");
    if (access.empty()) {
        result.error_message = "Invalid server response (no token).";
        return result;
    }
    storeTokens(access, refresh, remember_me);
    if (!fetchMeAndSetUser(access)) {
        result.error_message = "Login succeeded but could not load user.";
        return result;
    }
    result.success = true;
    result.user = current_user_;
    return result;
#else
    result.error_message = "Backend API requires Qt Network. Unset API URL for local mode.";
    return result;
#endif
}

RegisterResult UserAuthService::registerUser(const std::string& username, 
                                            const std::string& email, 
                                            const std::string& password) {
    RegisterResult result;

    if (auth_config_.isApiMode()) {
        return registerViaApi(username, email, password);
    }
    
    // Validate input (local mode)
    if (username.empty() || username.length() < 3) {
        result.error_message = "Username must be at least 3 characters long";
        return result;
    }
    
    if (email.empty() || email.find('@') == std::string::npos) {
        result.error_message = "Invalid email address";
        return result;
    }
    
    if (!validatePassword(password)) {
        result.error_message = "Password does not meet requirements (min 8 characters, at least one letter and one number)";
        return result;
    }
    
    // Check if user already exists
    if (userExists(username)) {
        result.error_message = "Username already exists";
        return result;
    }
    
    if (emailExists(email)) {
        result.error_message = "Email already registered";
        return result;
    }
    
    // Create new user
    User new_user;
    new_user.username = username;
    new_user.email = email;
    new_user.password_hash = hashPassword(password);
    new_user.created_date = getTimestamp();
    new_user.last_login_date = "";
    new_user.is_active = true;
    
    // Save user
    std::vector<User> users = loadUsers();
    users.push_back(new_user);
    
    if (saveUsers(users)) {
        result.success = true;
        result.user = new_user;
    } else {
        result.error_message = "Failed to save user data";
    }
    
    return result;
}

LoginResult UserAuthService::login(const std::string& username, const std::string& password, bool remember_me) {
    LoginResult result;

    // Test-Login (test/test) immer erlauben – für Testzwecke, auch wenn API-URL gesetzt ist
    if (username == "test" && password == "test") {
        result.success = true;
        result.user.username = "test";
        result.user.email = "test@local";
        result.user.created_date = "local";
        result.user.last_login_date = getTimestamp();
        result.user.is_active = true;
        current_user_ = result.user;
        is_logged_in_ = true;
        return result;
    }

    if (auth_config_.isApiMode()) {
        return loginViaApi(username, password, remember_me);
    }
    
    if (username.empty() || password.empty()) {
        result.error_message = "Username and password are required";
        return result;
    }
    
    std::vector<User> users = loadUsers();
    
    for (const auto& user : users) {
        if (user.username == username && user.is_active) {
            if (verifyPassword(password, user.password_hash)) {
                result.success = true;
                result.user = user;
                result.user.last_login_date = getTimestamp();
                updateLastLogin(username);
                current_user_ = result.user;
                is_logged_in_ = true;
                return result;
            } else {
                result.error_message = "Invalid password";
                return result;
            }
        }
    }
    
    result.error_message = "User not found";
    return result;
}

void UserAuthService::logout() {
    current_user_ = User();
    is_logged_in_ = false;
    clearStoredTokens();
    clearSavedSession();
}

bool UserAuthService::isLoggedIn() const {
    return is_logged_in_;
}

User UserAuthService::getCurrentUser() const {
    return current_user_;
}

void UserAuthService::updateLastLogin(const std::string& username) {
    std::vector<User> users = loadUsers();
    for (auto& user : users) {
        if (user.username == username) {
            user.last_login_date = getTimestamp();
            break;
        }
    }
    saveUsers(users);
}

bool UserAuthService::userExists(const std::string& username) const {
    std::vector<User> users = loadUsers();
    for (const auto& user : users) {
        if (user.username == username) {
            return true;
        }
    }
    return false;
}

bool UserAuthService::emailExists(const std::string& email) const {
    std::vector<User> users = loadUsers();
    for (const auto& user : users) {
        if (user.email == email) {
            return true;
        }
    }
    return false;
}

bool UserAuthService::validatePassword(const std::string& password) const {
    if (password.length() < 8) {
        return false;
    }
    
    bool has_letter = false;
    bool has_digit = false;
    
    for (char c : password) {
        if (std::isalpha(c)) {
            has_letter = true;
        } else if (std::isdigit(c)) {
            has_digit = true;
        }
    }
    
    return has_letter && has_digit;
}

void UserAuthService::saveSession(const std::string& username, bool remember) {
    #ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    if (remember) {
        settings.setValue("auth/remember_username", QString::fromStdString(username));
        settings.setValue("auth/remember_me", true);
    } else {
        settings.remove("auth/remember_username");
        settings.setValue("auth/remember_me", false);
    }
    settings.sync();
    #endif
}

bool UserAuthService::loadSavedSession() {
    #ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    if (auth_config_.isApiMode()) {
#ifdef CAD_USE_QT_NETWORK
        QString access = settings.value("auth/access_token").toString();
        if (!access.isEmpty() && fetchMeAndSetUser(access.toStdString())) {
            return true;
        }
        if (refreshAndSetUser()) {
            return true;
        }
#endif
        clearStoredTokens();
    }
    bool remember_me = settings.value("auth/remember_me", false).toBool();
    if (remember_me) {
        QString username = settings.value("auth/remember_username").toString();
        if (!username.isEmpty()) {
            return true;
        }
    }
    #endif
    return false;
}

void UserAuthService::clearSavedSession() {
    #ifdef CAD_USE_QT
    QSettings settings("HydraCAD", "HydraCAD");
    settings.remove("auth/remember_username");
    settings.setValue("auth/remember_me", false);
    settings.sync();
    #endif
}

}  // namespace app
}  // namespace cad
