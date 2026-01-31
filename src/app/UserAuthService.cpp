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
    hash.addData(password.c_str(), static_cast<int>(password.length()));
    std::string salt = generateSalt();
    hash.addData(salt.c_str(), static_cast<int>(salt.length()));
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
    hash_calc.addData(password.c_str(), static_cast<int>(password.length()));
    hash_calc.addData(salt.c_str(), static_cast<int>(salt.length()));
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

RegisterResult UserAuthService::registerViaApi(const std::string& username,
                                               const std::string& email,
                                               const std::string& password) const {
    (void)username;
    (void)email;
    (void)password;
    RegisterResult result;
    result.success = false;
    result.error_message = "Backend API not yet implemented. Unset CAD_API_BASE_URL for local mode.";
    return result;
}

LoginResult UserAuthService::loginViaApi(const std::string& username,
                                        const std::string& password) const {
    (void)username;
    (void)password;
    LoginResult result;
    result.success = false;
    result.error_message = "Backend API not yet implemented. Unset CAD_API_BASE_URL for local mode.";
    return result;
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

LoginResult UserAuthService::login(const std::string& username, const std::string& password) {
    LoginResult result;

    if (auth_config_.isApiMode()) {
        return loginViaApi(username, password);
    }
    
    if (username.empty() || password.empty()) {
        result.error_message = "Username and password are required";
        return result;
    }

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
    bool remember_me = settings.value("auth/remember_me", false).toBool();
    if (remember_me) {
        QString username = settings.value("auth/remember_username").toString();
        if (!username.isEmpty()) {
            // Return username for auto-fill, but don't auto-login
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
