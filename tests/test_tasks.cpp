#include <gtest/gtest.h>
#include "Task.h"
#include "TaskValidator.h"

static Task makeTask(const std::string& expected, const std::string& type = "output_match")
{
    Task t;
    t.id             = 1;
    t.title          = "Test";
    t.description    = "";
    t.hint           = "";
    t.type           = type;
    t.expectedOutput = expected;
    return t;
}

TEST(TaskValidator, CorrectAnswerPasses)
{
    auto t = makeTask("Hello World");
    auto r = TaskValidator::validate(t, "Hello World");
    EXPECT_TRUE(r.passed);
}

TEST(TaskValidator, WrongAnswerFails)
{
    auto t = makeTask("Hello World");
    auto r = TaskValidator::validate(t, "hello world");
    EXPECT_FALSE(r.passed);
    EXPECT_FALSE(r.feedback.empty());
}

TEST(TaskValidator, TrailingWhitespaceIgnored)
{
    auto t = makeTask("Hello World");
    auto r = TaskValidator::validate(t, "Hello World  \n");
    EXPECT_TRUE(r.passed);
}