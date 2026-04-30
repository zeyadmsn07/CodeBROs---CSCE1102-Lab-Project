#include "TaskLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Task> TaskLoader::load(const std::string& path)
{
    std::vector<Task> tasks;

    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "TaskLoader: could not open " << path << "\n";
        return tasks;
    }

    try {
        json arr = json::parse(in);
        for (auto& obj : arr) {
            Task t;
            t.id             = obj.value("id",             0);
            t.title          = obj.value("title",          "");
            t.description    = obj.value("description",    "");
            t.hint           = obj.value("hint",           "");
            t.type           = obj.value("type",           "output_match");
            t.expectedOutput = obj.value("expectedOutput", "");
            tasks.push_back(t);
        }
    }
    catch (...) {
        std::cerr << "TaskLoader: failed to parse " << path << "\n";
    }

    return tasks;
}