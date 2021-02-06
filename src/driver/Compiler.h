#ifndef COMPILER_H_
#define COMPILER_H_

#include <memory>
#include <string>

#include "driver/CompilerComponentsFactory.h"
#include "parser/Parser.h"

class Compiler {
public:
    Compiler(CompilerComponentsFactory compilerComponentsFactory);

    void compile(std::string sourceFileName) const;

private:
    CompilerComponentsFactory compilerComponentsFactory;
    std::unique_ptr<parser::Parser> parser;
};

#endif /* COMPILER_H_ */
