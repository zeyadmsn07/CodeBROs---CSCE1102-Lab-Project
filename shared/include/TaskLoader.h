#pragma once
#include "Task.h"
#include <string>
#include <vector>

class TaskLoader {
public:
    static std::vector<Task> load(const std::string& path);
};