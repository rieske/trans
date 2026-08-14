#ifndef DRIVER_RESOURCES_LOCATION_H_
#define DRIVER_RESOURCES_LOCATION_H_

#include <filesystem>
#include <string>

// Product resources live in <base>/resources/configuration/.
// Candidates: the executable directory, then its parent (build/trans -> repo root).
std::string resourcesBaseFromExecutableDir(const std::filesystem::path& executableDir);
std::string defaultResourcesBase();

#endif
