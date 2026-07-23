#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <string>

#include "driver/CompilerComponentsFactory.h"
#include "parser/Parser.h"


class Compiler {
public:
    Compiler(Configuration configuration);

    void compile(std::string sourceFileName) const;

private:
    std::string preprocess(const std::string& sourceFileName) const;
    void compileTranslationUnit(const std::string& sourceFileName, const std::string& assemblyFileName) const;

    Configuration configuration;
    CompilerComponentsFactory compilerComponentsFactory;
    parser::Grammar grammar;
    std::unique_ptr<parser::Parser> parser;
};

#endif // COMPILER_H_
