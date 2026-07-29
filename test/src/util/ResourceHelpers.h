#ifndef _RESOURCE_HELPERS_H_
#define _RESOURCE_HELPERS_H_

#include <string>

std::string getResourcesBaseDir();
std::string getResourcePath(std::string resource);
std::string getTestResourcePath(std::string resource);

// Write contents under test/programs/tmp/<name>.src (gitignored). Returns the path.
std::string writeTempSource(const std::string& name, const std::string& contents);

#endif
