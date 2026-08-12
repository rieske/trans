#ifndef UTIL_SOURCE_PATH_H_
#define UTIL_SOURCE_PATH_H_

#include <string>
#include <string_view>

namespace util {

enum class InputKind {
    Source,
    Preprocessed,
    Assembly,
    Object
};

inline bool hasSuffix(const std::string& path, std::string_view suffix) {
    return path.size() >= suffix.size()
            && path.compare(path.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Replace the final path extension (after the last slash) or append if none.
inline std::string withExtension(const std::string& path, const std::string& extension) {
    const auto slash = path.find_last_of("/\\");
    const std::size_t base = slash == std::string::npos ? 0 : slash + 1;
    const auto dot = path.find_last_of('.');
    if (dot != std::string::npos && dot > base) {
        return path.substr(0, dot) + extension;
    }
    return path + extension;
}

inline bool isObjectFile(const std::string& path) {
    return hasSuffix(path, ".o");
}

inline bool isAssemblyFile(const std::string& path) {
    return hasSuffix(path, ".s") || hasSuffix(path, ".S");
}

inline bool isPreprocessedFile(const std::string& path) {
    return hasSuffix(path, ".i");
}

inline InputKind classifyInput(const std::string& path) {
    if (isObjectFile(path)) {
        return InputKind::Object;
    }
    if (isAssemblyFile(path)) {
        return InputKind::Assembly;
    }
    if (isPreprocessedFile(path)) {
        return InputKind::Preprocessed;
    }
    return InputKind::Source;
}

} // namespace util

#endif
