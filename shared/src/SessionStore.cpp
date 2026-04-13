#include "SessionStore.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>

using json = nlohmann::json;

static long now() { return static_cast<long>(std::time(nullptr)); }

SessionStore::SessionStore(const std::string& filePath) : filePath_(filePath) {}

std::string SessionStore::generateToken() {
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<uint64_t> dist;

    std::ostringstream oss;
    for (int i = 0; i < 8; i++) oss << std::hex << std::setfill('0') << std::setw(16) << dist(rng);

    return oss.str();
}

std::string SessionStore::createSession(const std::string& username) {
    std::filesystem::create_directories(std::filesystem::path(filePath_).parent_path());
    std::string token = generateToken();
    long expiry = now() + 604800;  // 7 days

    json obj;
    obj["username"] = username;
    obj["token"] = token;
    obj["expiresAt"] = expiry;

    std::string tmp = filePath_ + ".tmp";
    std::ofstream out(tmp);
    out << obj.dump(2);
    out.close();
    std::filesystem::rename(tmp, filePath_);

    return token;
}

std::string SessionStore::checkSession() {
    std::ifstream in(filePath_);
    if (!in.is_open()) return "";

    try {
        json obj = json::parse(in);

        std::string username = obj.value("username", "");
        long expiry = obj.value("expiresAt", 0L);

        if (username.empty() || now() > expiry) return "";

        return username;
    } catch (...) {
        return "";
    }
}

void SessionStore::clearSession() {
    if (std::filesystem::exists(filePath_)) std::filesystem::remove(filePath_);
}
