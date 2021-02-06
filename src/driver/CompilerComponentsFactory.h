#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include "Configuration.h"
#include "driver/Configuration.h"

#include <iostream>
#include <memory>
#include <string>

namespace codegen {
class AssemblyGenerator;
} /* namespace codegen */

namespace semantic_analyzer {
class SemanticAnalyzer;
} /* namespace semantic_analyzer */

namespace parser {
class Parser;
class SyntaxTreeBuilder;
} /* namespace parser */

class Scanner;

class CompilerComponentsFactory {
public:
    CompilerComponentsFactory(Configuration configuration);

    std::unique_ptr<Scanner> makeScannerForSourceFile(std::string sourceFileName) const;

    std::unique_ptr<parser::Parser> makeParser() const;
    std::unique_ptr<parser::SyntaxTreeBuilder> makeSyntaxTreeBuilder(std::string sourceFileName) const;

    std::unique_ptr<codegen::AssemblyGenerator> makeAssemblyGenerator(std::ostream* assemblyFile) const;

private:
    Configuration configuration;
};

#endif /* COMPILERCOMPONENTSFACTORY_H_ */
