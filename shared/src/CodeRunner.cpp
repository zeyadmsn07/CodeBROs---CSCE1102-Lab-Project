#include "CodeRunner.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

static std::string readFile(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string runCommand(const std::string& cmd)
{
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buf[256];
    while (fgets(buf, sizeof(buf), pipe))
        result += buf;

    pclose(pipe);
    return result;
}

RunResult CodeRunner::run(const std::string& code)
{
    RunResult res;

    std::string srcPath = "/tmp/codebros_task.cpp";
    std::string binPath = "/tmp/codebros_task_bin";
    std::string errPath = "/tmp/codebros_task_err.txt";

    {
        std::ofstream out(srcPath);
        if (!out.is_open()) {
            res.errorMsg = "Could not write temp file.";
            return res;
        }
        out << code;
    }

    // compile — redirect stderr to errPath
    std::string compileCmd =
        "g++ -std=c++17 -o " + binPath +
        " " + srcPath +
        " 2>" + errPath;

    int compileRet = std::system(compileCmd.c_str());

    if (compileRet != 0) {
        res.success  = false;
        res.errorMsg = readFile(errPath);
        if (res.errorMsg.empty())
            res.errorMsg = "Compilation failed.";
        return res;
    }

    // run with a 5-second timeout so infinite loops don't hang the app
    std::string runCmd = "timeout 5 " + binPath + " 2>/dev/null";
    res.output  = runCommand(runCmd);
    res.success = true;

    std::filesystem::remove(srcPath);
    std::filesystem::remove(binPath);
    std::filesystem::remove(errPath);

    return res;
}