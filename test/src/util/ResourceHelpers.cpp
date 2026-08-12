#include "ResourceHelpers.h"

#include <cerrno>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>
#include <utility>

std::string getResourcesBaseDir() {
    return "../../../";
}

std::string getResourcePath(std::string resource) {
    return "../../../resources/" + resource;
}

std::string getTestResourcePath(std::string resource) {
    return "../../../test/" + resource;
}

std::string writeTempSource(const std::string& name, const std::string& contents) {
    const std::string dir = getTestResourcePath("programs/tmp/");
    if (mkdir(dir.c_str(), 0777) == -1 && errno != EEXIST) {
        throw std::runtime_error("Could not create " + dir);
    }
    const std::string path = dir + name + ".c";
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Could not write temp source " + path);
    }
    out << contents;
    return path;
}

ScopedTempFile::ScopedTempFile(std::string path) :
        path_ { std::move(path) } {
}

ScopedTempFile::~ScopedTempFile() {
    std::remove(path_.c_str());
}

const std::string& ScopedTempFile::path() const {
    return path_;
}
