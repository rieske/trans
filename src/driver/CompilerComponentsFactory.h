#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include <memory>
#include <string>

namespace parser {
class Parser;
class SyntaxTreeBuilder;
} /* namespace parser */

class SemanticAnalyzer;
class Configuration;
class Scanner;

class CompilerComponentsFactory {
public:
	CompilerComponentsFactory(const Configuration& configuration);
	virtual ~CompilerComponentsFactory();

	std::unique_ptr<Scanner> scannerForSourceFile(std::string sourceFileName, std::string scannerConfigurationFileName =
			defaultScannerConfigurationFileName) const;
	std::unique_ptr<parser::Parser> getParser() const;
	std::unique_ptr<parser::SyntaxTreeBuilder> newSyntaxTreeBuilder() const;
	std::unique_ptr<SemanticAnalyzer> newSemanticAnalyzer() const;

private:
	const Configuration& configuration;

	static const std::string defaultScannerConfigurationFileName;
};

#endif /* COMPILERCOMPONENTSFACTORY_H_ */
