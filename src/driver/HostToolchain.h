#ifndef HOST_TOOLCHAIN_H_
#define HOST_TOOLCHAIN_H_

#include <string>
#include <vector>

// Shared helpers for host gcc/nasm integration (link-only and full compile).

std::string shellQuote(const std::string& s);

// Fixed host gcc -E flags. Split so defines that must override user -D/-U
// can be placed after them (see Compiler::preprocess).
// Dialect: -E -P -std=c99 -x c
std::string hostGccPreprocessDialectFlags();
// Trailing: -D__STDC__=0 -DCURL_DISABLE_TYPECHECK -w
std::string hostGccPreprocessTrailingFlags();

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

// Build the host link command (does not run it). Returns empty string if
// va_tls.o cannot be found.
std::string buildHostLinkCommand(const std::vector<std::string>& objectFiles,
        const std::string& outputFile,
        const std::string& resourcesBasePath,
        const std::vector<std::string>& extraLinkArgs = {},
        VaTlsSearchOptions options = {});

#endif // HOST_TOOLCHAIN_H_
