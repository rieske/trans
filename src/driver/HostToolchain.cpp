#include "HostToolchain.h"

#include <sys/stat.h>

#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::string> hostGccPreprocessDialectArgv() {
    // -E -P: preprocess only, no linemarkers.
    // -std=c99 -x c: C99 dialect; accept non-.c paths (functional .src).
    return { "-E", "-P", "-std=c99", "-x", "c" };
}

std::vector<std::string> hostGccPreprocessTrailingArgv() {
    // Applied after user -D/-U so they win over user overrides of the same names.
    return { "-D__STDC__=0", "-DCURL_DISABLE_TYPECHECK", "-w" };
}

bool fileExistsNonEmpty(const std::string& path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode) && st.st_size > 0;
}

bool fileExists(const std::string& path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

std::string parentDirectory(std::string path) {
    while (path.size() > 1 && path.back() == '/') {
        path.pop_back();
    }
    if (path.empty() || path == "/") {
        return {};
    }
    auto slash = path.find_last_of('/');
    if (slash == std::string::npos) {
        return {};
    }
    if (slash == 0) {
        return "/";
    }
    return path.substr(0, slash);
}

std::string findFileWalkingUp(std::string startDir,
        const std::vector<std::string>& relativeCandidates,
        int maxLevels,
        PathExistsFn exists) {
    if (!exists || relativeCandidates.empty() || maxLevels < 0) {
        return {};
    }
    std::string dir = std::move(startDir);
    for (int i = 0; i < maxLevels && !dir.empty(); ++i) {
        while (dir.size() > 1 && dir.back() == '/') {
            dir.pop_back();
        }
        for (const auto& rel : relativeCandidates) {
            std::string candidate = (dir == "/") ? ("/" + rel) : (dir + "/" + rel);
            if (exists(candidate)) {
                return candidate;
            }
        }
        if (dir == "/") {
            break;
        }
        dir = parentDirectory(dir);
    }
    return {};
}

std::string findDirWalkingUp(std::string startDir,
        const std::string& markerRelative,
        int maxLevels,
        PathExistsFn exists) {
    std::string found = findFileWalkingUp(std::move(startDir), { markerRelative }, maxLevels, exists);
    if (found.empty()) {
        return {};
    }
    // Strip "/markerRelative" to get the directory that contains it.
    if (found.size() > markerRelative.size()
            && found.compare(found.size() - markerRelative.size(), markerRelative.size(),
                    markerRelative) == 0) {
        std::string dir = found.substr(0, found.size() - markerRelative.size());
        while (dir.size() > 1 && dir.back() == '/') {
            dir.pop_back();
        }
        if (dir.empty()) {
            return "/";
        }
        if (dir.back() != '/') {
            dir.push_back('/');
        }
        return dir;
    }
    return {};
}

void ensureNonEmptyObjectFile(const std::string& path) {
    if (!fileExistsNonEmpty(path)) {
        throw std::runtime_error("empty object " + path);
    }
}

std::vector<std::string> buildHostLinkArgv(const std::vector<std::string>& objectFiles,
        const std::string& outputFile,
        const std::vector<std::string>& extraLinkArgs) {
    std::vector<std::string> argv { "gcc", "-m64", "-pie" };
    if (!outputFile.empty()) {
        argv.push_back("-o");
        argv.push_back(outputFile);
    }
    for (const auto& obj : objectFiles) {
        argv.push_back(obj);
    }
    for (const auto& arg : extraLinkArgs) {
        argv.push_back(arg);
    }
    return argv;
}


