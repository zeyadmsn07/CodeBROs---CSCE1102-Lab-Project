#pragma once
#include <string>

struct RunResult {
    bool        success = false;
    std::string output;
    std::string errorMsg;
};

class CodeRunner {
public:
    static RunResult run(const std::string& code);
};