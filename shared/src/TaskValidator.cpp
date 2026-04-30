#include "TaskValidator.h"
#include <algorithm>

std::string TaskValidator::normalize(const std::string& s)
{
    std::string out = s;
    out.erase(std::remove(out.begin(), out.end(), '\r'), out.end());
    while (!out.empty() && (out.back() == ' ' || out.back() == '\n'))
        out.pop_back();
    return out;
}

ValidationResult TaskValidator::validate(const Task& task,
                                         const std::string& submission)
{
    std::string expected = normalize(task.expectedOutput);
    std::string actual   = normalize(submission);

    if (actual == expected)
        return { true, "Correct!" };

    return { false, "Expected:\n" + expected + "\n\nGot:\n" + actual };
}