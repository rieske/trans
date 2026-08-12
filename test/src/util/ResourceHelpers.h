#ifndef _RESOURCE_HELPERS_H_
#define _RESOURCE_HELPERS_H_

#include <string>

std::string getResourcesBaseDir();
std::string getResourcePath(std::string resource);
std::string getTestResourcePath(std::string resource);

std::string writeTempSource(const std::string& name, const std::string& contents);

class ScopedTempFile {
public:
    explicit ScopedTempFile(std::string path);
    ~ScopedTempFile();

    ScopedTempFile(const ScopedTempFile&) = delete;
    ScopedTempFile& operator=(const ScopedTempFile&) = delete;

    const std::string& path() const;

private:
    std::string path_;
};

#endif
