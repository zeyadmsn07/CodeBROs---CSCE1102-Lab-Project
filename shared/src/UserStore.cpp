#include "UserStore.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "picosha2.h"

using json = nlohmann::json;

static long now() { return static_cast<long>(std::time(nullptr)); }
UserStore::UserStore(const std::string& filePath) : filePath_(filePath) {
    // Create file with empty array if it does not exist
    if (!std::filesystem::exists(filePath_)) {
        std::filesystem::create_directories(std::filesystem::path(filePath_).parent_path());
        std::ofstream out(filePath_);
        out << "[]";
    }
    loadFromFile();
}

void UserStore::loadFromFile() {
    std::ifstream in(filePath_);
    if (!in.is_open()) return;

    try {
        json arr = json::parse(in);
        users_.clear();
        for (auto& obj : arr) {
            UserRecord r;
            r.username = obj.value("username", "");
            r.passwordHash = obj.value("passwordHash", "");
            r.createdAt = obj.value("createdAt", 0L);
            r.lastLogin = obj.value("lastLogin", 0L);
            users_.push_back(r);
        }
    } catch (...) {
        users_.clear();
    }
}

std::string UserStore::hashPassword(const std::string& password) const {
    return picosha2::hash256_hex_string(password);
}

bool UserStore::registerUser(const std::string& username, const std::string& password) {
    // Check for duplicate username
    auto it = std::find_if(users_.begin(), users_.end(),
                           [&](const UserRecord& r) { return r.username == username; });

    if (it != users_.end()) return false;  // username already taken

    UserRecord r;
    r.username = username;
    r.passwordHash = hashPassword(password);
    r.createdAt = now();
    r.lastLogin = now();
    users_.push_back(r);

    saveToFile();
    return true;
}

bool UserStore::authenticate(const std::string& username, const std::string& password) {
    auto it = std::find_if(users_.begin(), users_.end(),
                           [&](const UserRecord& r) { return r.username == username; });

    if (it == users_.end()) return false;

    return it->passwordHash == hashPassword(password);
}

void UserStore::updateLastLogin(const std::string& username) {
    auto it = std::find_if(users_.begin(), users_.end(),
                           [&](const UserRecord& r) { return r.username == username; });

    if (it == users_.end()) return;

    it->lastLogin = now();
    saveToFile();
}

void UserStore::saveToFile() {
    json arr = json::array();
    for (auto& u : users_) {
        arr.push_back({{"username", u.username},
                       {"passwordHash", u.passwordHash},
                       {"createdAt", u.createdAt},
                       {"lastLogin", u.lastLogin}});
    }
    std::string tmp = filePath_ + ".tmp";
    std::ofstream out(tmp);
    out << arr.dump(2);
    out.close();
    std::filesystem::rename(tmp, filePath_);
}