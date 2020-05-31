#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <string>

#include "parser/Parser.h"

class CompilerComponentsFactory;

class Compiler {
public:
    Compiler(std::unique_ptr<CompilerComponentsFactory> compilerComponentsFactory);

    void compile(std::string sourceFileName) const;

private:
    std::unique_ptr<CompilerComponentsFactory> compilerComponentsFactory;
    std::unique_ptr<parser::Parser> parser;
};

#endif /* COMPILER_H_ */
