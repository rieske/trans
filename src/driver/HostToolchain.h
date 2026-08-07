#ifndef HOST_TOOLCHAIN_H_
#define HOST_TOOLCHAIN_H_

#include <string>
#include <vector>

// Shared helpers for host gcc/nasm integration (link-only and full compile).
// Product exec uses Argv + util::Process only (no shell command strings).

bool fileExistsNonEmpty(const std::string& path);

// True if path can be opened as a file (empty files count as present).
bool fileExists(const std::string& path);

// Parent directory of path, or empty if none. Root "/" yields empty.
std::string parentDirectory(std::string path);

// Walk startDir and its ancestors (inclusive), up to maxLevels parent steps.
// At each directory, try each relative candidate; return the first full path
// for which exists(path) is true. Returns empty if none match.
using PathExistsFn = bool (*)(const std::string&);
std::string findFileWalkingUp(std::string startDir,
        const std::vector<std::string>& relativeCandidates,
        int maxLevels,
        PathExistsFn exists);

// Directory (with trailing slash) that contains markerRelative, walking up.
std::string findDirWalkingUp(std::string startDir,
        const std::string& markerRelative,
        int maxLevels,
        PathExistsFn exists = fileExists);

// After nasm: make must not continue with a missing/empty .o (former trans-cc).
// Throws std::runtime_error whose what() starts with "empty object ".
void ensureNonEmptyObjectFile(const std::string& path);

// Locate va_tls.o. Order: TRANS_VA_TLS env, resourcesBasePath, optional
// /proc/self/exe and cwd walks (disable in unit tests for isolation).
struct VaTlsSearchOptions {
    bool searchExe { true };
    bool searchCwd { true };
};

std::string findVaTlsObject(const std::string& resourcesBasePath,
        VaTlsSearchOptions options = {});

// Argv form of the host link (no shell). Empty if va_tls.o cannot be found.
// Product exec uses this with util::Process only (no shell command string).
std::vector<std::string> buildHostLinkArgv(const std::vector<std::string>& objectFiles,
        const std::string& outputFile,
        const std::string& resourcesBasePath,
        const std::vector<std::string>& extraLinkArgs = {},
        VaTlsSearchOptions options = {});

// Fixed host gcc -E dialect / trailing flags as argv fragments.
std::vector<std::string> hostGccPreprocessDialectArgv();
std::vector<std::string> hostGccPreprocessTrailingArgv();

#endif // HOST_TOOLCHAIN_H_
