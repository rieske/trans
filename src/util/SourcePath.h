#ifndef UTIL_SOURCE_PATH_H_
#define UTIL_SOURCE_PATH_H_

#include <string>
#include <string_view>

namespace util {

// Driver classification of positional inputs.
enum class InputKind {
    Source,
    Preprocessed,
    Assembly,
    // Objects, archives, and shared libraries: pass through to the link step.
    LinkInput
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

// Relocatable objects, static archives, and shared libraries (incl. soname).
inline bool isLinkInput(const std::string& path) {
    if (hasSuffix(path, ".o") || hasSuffix(path, ".a") || hasSuffix(path, ".so")) {
        return true;
    }
    const auto slash = path.find_last_of("/\\");
    const std::string_view name = slash == std::string::npos
            ? std::string_view { path }
            : std::string_view { path }.substr(slash + 1);
    const auto pos = name.find(".so.");
    return pos != std::string_view::npos && pos + 4 < name.size()
            && name[pos + 4] >= '0' && name[pos + 4] <= '9';
}

inline bool isAssemblyFile(const std::string& path) {
    return hasSuffix(path, ".s") || hasSuffix(path, ".S");
}

inline bool isPreprocessedFile(const std::string& path) {
    return hasSuffix(path, ".i");
}

inline InputKind classifyInput(const std::string& path) {
    if (isLinkInput(path)) {
        return InputKind::LinkInput;
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
