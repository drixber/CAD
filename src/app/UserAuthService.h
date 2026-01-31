#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include "AuthConfig.h"
#include "HttpClient.h"

namespace cad {
namespace app {

struct User {
    std::string username;
    std::string email;
    std::string password_hash;  // Hashed password
    std::string created_date;
    std::string last_login_date;
    bool is_active{true};
};

struct LoginResult {
    bool success{false};
    std::string error_message;
    User user;
};

struct RegisterResult {
    bool success{false};
    std::string error_message;
    User user;
};

class UserAuthService {
public:
    UserAuthService();
    ~UserAuthService();
    
    // Registration
    RegisterResult registerUser(const std::string& username, 
                               const std::string& email, 
                               const std::string& password);
    
    // Login
    LoginResult login(const std::string& username, const std::string& password);
    
    // Logout
    void logout();
    
    // Session management
    bool isLoggedIn() const;
    User getCurrentUser() const;
    void updateLastLogin(const std::string& username);
    
    // User management
    bool userExists(const std::string& username) const;
    bool emailExists(const std::string& email) const;
    bool validatePassword(const std::string& password) const;
    
    // Remember me
    void saveSession(const std::string& username, bool remember);
    bool loadSavedSession();
    void clearSavedSession();

    // Backend API config: when set, register/login use API instead of local file
    std::string getApiBaseUrl() const;
    void setApiBaseUrl(const std::string& url);

private:
    AuthConfig auth_config_;
    mutable HttpClient http_client_;
    std::string users_file_path_;
    User current_user_;
    bool is_logged_in_{false};
    std::string access_token_;
    std::string refresh_token_;

    RegisterResult registerViaApi(const std::string& username, const std::string& email, const std::string& password);
    LoginResult loginViaApi(const std::string& username, const std::string& password);
    bool fetchMeAndSetUser(const std::string& access_token);
    bool refreshAndSetUser();
    void storeTokens(const std::string& access, const std::string& refresh, bool remember);
    void clearStoredTokens();

    std::string hashPassword(const std::string& password) const;
    bool verifyPassword(const std::string& password, const std::string& hash) const;
    std::vector<User> loadUsers() const;
    bool saveUsers(const std::vector<User>& users) const;
    std::string getTimestamp() const;
    std::string generateSalt() const;
};

}  // namespace app
}  // namespace cad
