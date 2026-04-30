#pragma once
#include <string>

struct Task {
    int         id = 0;
    std::string title;
    std::string description;
    std::string hint;
    std::string type;
    std::string expectedOutput;
};

struct ValidationResult {
    bool        passed = false;
    std::string feedback;
};