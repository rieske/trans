#include "ResourcesLocation.h"

namespace {

bool hasResourcesTree(const std::filesystem::path& dir) {
    return std::filesystem::is_regular_file(
            dir / "resources" / "configuration" / "grammar.bnf");
}

std::string withTrailingSlash(std::filesystem::path dir) {
    std::string path = dir.lexically_normal().string();
    if (!path.empty() && path.back() != '/') {
        path += '/';
    }
    return path;
}

} // namespace

std::string resourcesBaseFromExecutableDir(const std::filesystem::path& executableDir) {
    if (executableDir.empty()) {
        return {};
    }
    if (hasResourcesTree(executableDir)) {
        return withTrailingSlash(executableDir);
    }
    const auto parent = executableDir.parent_path();
    if (parent != executableDir && hasResourcesTree(parent)) {
        return withTrailingSlash(parent);
    }
    return {};
}

std::string defaultResourcesBase() {
    std::error_code ec;
    const auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec) {
        return {};
    }
    return resourcesBaseFromExecutableDir(exe.parent_path());
}
