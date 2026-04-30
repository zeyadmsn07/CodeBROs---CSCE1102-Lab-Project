#include "TaskValidator.h"
#include "CodeRunner.h"
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
                                         const std::string& code)
{
    RunResult run = CodeRunner::run(code);

    if (!run.success) {
        return {
            false,
            "Compilation error:\n" + run.errorMsg
        };
    }

    std::string expected = normalize(task.expectedOutput);
    std::string actual   = normalize(run.output);

    if (actual == expected)
        return { true, "Correct!" };

    return {
        false,
        "Your program compiled and ran, but the output was wrong.\n\n"
        "Expected:\n" + expected + "\n\n"
        "Got:\n" + actual
    };
}