#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <string>
#include <vector>

#include "driver/CompilerComponentsFactory.h"
#include "parser/Parser.h"

class Compiler {
public:
    Compiler(Configuration configuration);

    std::string compile(std::string sourceFileName) const;
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
    CompilerComponentsFactory compilerComponentsFactory;
    std::shared_ptr<const LanguageFrontEnd> frontEnd;
    std::unique_ptr<parser::Parser> parser;
};

#endif // COMPILER_H_
