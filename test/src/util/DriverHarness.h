#ifndef DRIVER_HARNESS_H_
#define DRIVER_HARNESS_H_

#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"
#include "util/LogManager.h"
#include "ResourceHelpers.h"

#include <sstream>
#include <string>
#include <vector>

// argv strings must outlive ConfigurationParser (it does not copy flags).
struct ArgvBuffer {
    std::string executable { "trans" };
    std::string resourcesFlag;
    std::vector<std::string> extraFlags;
    std::vector<std::string> files;
    std::vector<char*> pointers;

    explicit ArgvBuffer(std::vector<std::string> paths, std::vector<std::string> flags = {}) :
            resourcesFlag { "-r" + getResourcesBaseDir() },
            extraFlags { std::move(flags) },
            files { std::move(paths) } {
        pointers.push_back(executable.data());
        pointers.push_back(resourcesFlag.data());
        for (auto& flag : extraFlags) {
            pointers.push_back(flag.data());
        }
        for (auto& file : files) {
            pointers.push_back(file.data());
        }
    }

    int argc() const {
        return static_cast<int>(pointers.size());
    }

    char** argv() {
        return pointers.data();
    }
};

inline int runDriver(ArgvBuffer& args, std::string* errorOutput = nullptr) {
    std::stringstream outputStream;
    std::stringstream errorStream;
    int exitCode = 0;
    LogManager::withOutputStreams(outputStream, errorStream, [&]() {
        Driver driver {};
        exitCode = driver.run(ConfigurationParser { args.argc(), args.argv() });
    });
    if (errorOutput != nullptr) {
        *errorOutput = errorStream.str();
    }
    return exitCode;
}

inline std::string transBinaryPath() {
#ifdef TRANS_BINARY
    return TRANS_BINARY;
#else
    return "../../trans";
#endif
}

#endif
