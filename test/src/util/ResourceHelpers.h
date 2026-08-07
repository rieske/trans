#ifndef _RESOURCE_HELPERS_H_
#define _RESOURCE_HELPERS_H_

#include <string>

std::string getResourcesBaseDir();
std::string getResourcePath(std::string resource);
std::string getTestResourcePath(std::string resource);

// Directory for generated functional/scanner temp sources (gitignored).
// Always ends with a trailing slash.
std::string getTestProgramsTmpDir();
// Create test/programs/tmp/ if missing (clean CI checkouts have no tmp/).
void ensureTestProgramsTmpDir();

#endif
