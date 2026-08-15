#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include "Configuration.h"
#include "codegen/AssemblyGenerator.h"
#include "driver/LanguageFrontEnd.h"
#include "parser/Grammar.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"

#include <iostream>
#include <memory>
#include <string>

class CompilerComponentsFactory {
public:
    CompilerComponentsFactory(Configuration configuration);

    std::unique_ptr<scanner::Scanner> makeScannerForSourceFile(
            std::string sourceFileName, scanner::LexicalSession& session) const;

    std::shared_ptr<const LanguageFrontEnd> makeFrontEnd() const;
    std::unique_ptr<parser::SyntaxTreeBuilder> makeSyntaxTreeBuilder(const parser::Grammar* grammar, scanner::LexicalSession& session) const;

    std::unique_ptr<codegen::AssemblyGenerator> makeAssemblyGenerator(std::ostream* assemblyFile) const;

private:
    Configuration configuration;
};

#endif // COMPILERCOMPONENTSFACTORY_H_
