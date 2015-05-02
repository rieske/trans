#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

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

class Configuration;
class Scanner;

class CompilerComponentsFactory {
public:
    CompilerComponentsFactory(const Configuration& configuration);
    virtual ~CompilerComponentsFactory() = default;

    std::unique_ptr<Scanner> makeScannerForSourceFile(std::string sourceFileName, std::string scannerConfigurationFileName =
            defaultScannerConfigurationFileName) const;

    std::unique_ptr<parser::Parser> makeParser() const;
    std::unique_ptr<parser::SyntaxTreeBuilder> makeSyntaxTreeBuilder() const;

    std::unique_ptr<codegen::AssemblyGenerator> makeAssemblyGenerator(std::ostream* assemblyFile) const;

private:
    const Configuration& configuration;

    static const std::string defaultScannerConfigurationFileName;
};

#endif /* COMPILERCOMPONENTSFACTORY_H_ */
