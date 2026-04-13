#ifndef SESSIONSTORE_H
#define SESSIONSTORE_H

#include <string>

class SessionStore {
   public:
    explicit SessionStore(const std::string& filePath);

    std::string createSession(const std::string& username);
    std::string checkSession();
    void clearSession();

   private:
    std::string filePath_;

    std::string generateToken();
};

#endif
