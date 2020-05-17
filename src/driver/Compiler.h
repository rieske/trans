#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <string>

#include "parser/Parser.h"

class CompilerComponentsFactory;

class Compiler {
public:
    Compiler(const CompilerComponentsFactory* compilerComponentsFactory);

    void compile(std::string sourceFileName) const;

private:
    std::unique_ptr<const CompilerComponentsFactory> compilerComponentsFactory;
    std::unique_ptr<parser::Parser> parser;
};

#endif /* COMPILER_H_ */
