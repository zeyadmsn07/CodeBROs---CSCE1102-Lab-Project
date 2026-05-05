#pragma once
#include "Task.h"
#include "CodeRunner.h"
#include <string>

class TaskValidator {
public:
    static ValidationResult validate(const Task& task, const std::string& code);
private:
    static std::string normalize(const std::string& s);
};