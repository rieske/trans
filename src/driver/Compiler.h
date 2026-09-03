#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Configuration.h"
#include "driver/LanguageFrontEnd.h"

class Compiler {
public:
    Compiler(Configuration configuration);

    std::optional<std::string> compile(std::string sourceFileName) const;
    static std::string assembleFile(std::string assemblyFileName, const Configuration& configuration);
    static std::vector<std::string> linkCommand(const std::vector<std::string>& objectFiles,
            const std::string& executableFileName,
            const std::vector<std::string>& linkerArgs = {});
    static void link(const std::vector<std::string>& objectFiles, const std::string& executableFileName,
            const std::vector<std::string>& linkerArgs = {});
    static std::string defaultExecutablePath(const std::string& sourceFileName);
    static std::vector<std::string> preprocessCommand(const std::string& sourceFileName,
            const std::string& outputPath, const Configuration& configuration);
    static std::vector<std::string> preprocessCommand(const std::vector<std::string>& sourceFileNames,
            const std::string& outputPath, const Configuration& configuration);
    static bool sourceFileNeedsGccPreprocessor(const std::string& sourceFileName,
            const Configuration& configuration);

private:
    Configuration configuration;
    std::shared_ptr<const LanguageFrontEnd> frontEnd;
};

#endif // COMPILER_H_
