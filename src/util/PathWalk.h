#ifndef UTIL_PATH_WALK_H_
#define UTIL_PATH_WALK_H_

#include <string>
#include <vector>

namespace util {

bool fileExistsNonEmpty(const std::string& path);
bool fileExists(const std::string& path);

// Parent directory of path, or empty if none. Root "/" yields empty.
std::string parentDirectory(std::string path);

using PathExistsFn = bool (*)(const std::string&);

// Walk startDir and ancestors (inclusive), up to maxLevels parent steps.
std::string findFileWalkingUp(std::string startDir,
        const std::vector<std::string>& relativeCandidates,
        int maxLevels,
        PathExistsFn exists);

// Directory (with trailing slash) that contains markerRelative, walking up.
std::string findDirWalkingUp(std::string startDir,
        const std::string& markerRelative,
        int maxLevels,
        PathExistsFn exists = fileExists);

} // namespace util

#endif
