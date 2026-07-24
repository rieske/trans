#include "ResourceHelpers.h"

#include <cerrno>
#include <cstring>
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

std::string getTestProgramsTmpDir() {
    return getTestResourcePath("programs/tmp/");
}

void ensureTestProgramsTmpDir() {
    const std::string dir = getTestProgramsTmpDir();
    if (mkdir(dir.c_str(), 0777) == -1 && errno != EEXIST) {
        throw std::runtime_error("Could not create directory " + dir + ": "
                + std::to_string(errno) + ":" + std::strerror(errno));
    }
}
