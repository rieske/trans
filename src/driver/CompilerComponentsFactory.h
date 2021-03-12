#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include "Configuration.h"
#include "codegen/AssemblyGenerator.h"
#include "driver/Configuration.h"
#include "parser/Grammar.h"
#include "parser/Parser.h"
#include "parser/ParsingTable.h"
#include "scanner/Scanner.h"

#include <iostream>
#include <memory>
#include <string>

class CompilerComponentsFactory {
public:
    CompilerComponentsFactory(Configuration configuration);

    std::unique_ptr<scanner::Scanner> makeScannerForSourceFile(std::string sourceFileName) const;

    parser::Grammar makeGrammar() const;
    std::unique_ptr<parser::Parser> makeParser(parser::Grammar* grammar) const;
    std::unique_ptr<parser::SyntaxTreeBuilder> makeSyntaxTreeBuilder(std::string sourceFileName, const parser::Grammar* grammar) const;

    std::unique_ptr<codegen::AssemblyGenerator> makeAssemblyGenerator(std::ostream* assemblyFile) const;

    parser::ParsingTable* generateParsingTable(const parser::Grammar* grammar) const;

private:
    Configuration configuration;
};

#endif // COMPILERCOMPONENTSFACTORY_H_
