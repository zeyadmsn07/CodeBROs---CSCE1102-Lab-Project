#include <gtest/gtest.h>
#include "UserStore.h"
#include "SessionStore.h"
#include <filesystem>
#include <fstream>

// wipe test files before and after each test
class AuthTest : public ::testing::Test {
protected:
    const std::string usersFile    = "/tmp/test_users.json";
    const std::string sessionsFile = "/tmp/test_sessions.json";

    void SetUp() override {
        // Start with an empty users file
        std::ofstream(usersFile) << "[]";
        std::filesystem::remove(sessionsFile);
    }

    void TearDown() override {
        std::filesystem::remove(usersFile);
        std::filesystem::remove(sessionsFile);
    }
};


TEST_F(AuthTest, RegisterNewUser)
{
    UserStore store(usersFile);
    EXPECT_TRUE(store.registerUser("Zeyad", "shawarma123"));
}

TEST_F(AuthTest, PreventDuplicateUsernames)
{
    UserStore store(usersFile);
    store.registerUser("Zeyad", "shawarma123");

    EXPECT_FALSE(store.registerUser("Zeyad", "anotherPass"));
}

TEST_F(AuthTest, AcceptCorrectPassword)
{
    UserStore store(usersFile);
    store.registerUser("Zeyad", "shawarma123");

    EXPECT_TRUE(store.authenticate("Zeyad", "shawarma123"));
}

TEST_F(AuthTest, RejectIncorrectPassword)
{
    UserStore store(usersFile);
    store.registerUser("Zeyad", "shawarma123");

    EXPECT_FALSE(store.authenticate("Zeyad", "wrongpass"));
}

TEST_F(AuthTest, RejectNonexistentUser)
{
    UserStore store(usersFile);

    EXPECT_FALSE(store.authenticate("UnknownUser", "shawarma123"));
}

TEST_F(AuthTest, SessionIsCreatedAndRetrieved)
{
    SessionStore sessions(sessionsFile);

    sessions.createSession("Zeyad");
    EXPECT_EQ(sessions.checkSession(), "Zeyad");
}

TEST_F(AuthTest, NoSessionReturnsEmptyString)
{
    SessionStore sessions(sessionsFile);

    EXPECT_EQ(sessions.checkSession(), "");
}

TEST_F(AuthTest, ClearingSessionRemovesIt)
{
    SessionStore sessions(sessionsFile);

    sessions.createSession("Zeyad");
    sessions.clearSession();

    EXPECT_EQ(sessions.checkSession(), "");
}