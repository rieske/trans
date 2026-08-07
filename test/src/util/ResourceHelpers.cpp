#include "ResourceHelpers.h"

#include <cerrno>
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>

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
    const std::string path = dir + name + ".src";
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Could not write temp source " + path);
    }
    out << contents;
    return path;
}
