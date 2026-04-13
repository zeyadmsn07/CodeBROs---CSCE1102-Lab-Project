#pragma once
#include <string>
#include <vector>

struct UserRecord {
    std::string username;
    std::string passwordHash;
    long createdAt = 0;
    long lastLogin = 0;
};

class UserStore {
   public:
    explicit UserStore(const std::string& filePath);

    bool registerUser(const std::string& username, const std::string& password);
    bool authenticate(const std::string& username, const std::string& password);
    void updateLastLogin(const std::string& username);
    void saveToFile();

   private:
    std::string filePath_;
    std::vector<UserRecord> users_;

    void loadFromFile();
    std::string hashPassword(const std::string& password) const;
};