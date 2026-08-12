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
    static void link(const std::vector<std::string>& objectFiles, const std::string& executableFileName);
    static std::string defaultExecutablePath(const std::string& sourceFileName);
    static std::vector<std::string> preprocessCommand(const std::string& sourceFileName,
            const std::string& outputPath, const Configuration& configuration);

private:
    Configuration configuration;
    CompilerComponentsFactory compilerComponentsFactory;
    parser::Grammar grammar;
    std::unique_ptr<parser::Parser> parser;
};

#endif // COMPILER_H_
