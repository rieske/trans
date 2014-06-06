#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include <memory>
#include <string>

class Configuration;
class SemanticComponentsFactory;

class Parser;
class Scanner;

class CompilerComponentsFactory {
public:
	CompilerComponentsFactory(const Configuration& configuration);
	virtual ~CompilerComponentsFactory();

	std::unique_ptr<Scanner> getScanner(std::string sourceFileName) const;
	std::unique_ptr<Parser> getParser() const;

private:
	SemanticComponentsFactory* newSemanticComponentsFactory() const;

	const Configuration& configuration;
};

#endif /* COMPILERCOMPONENTSFACTORY_H_ */
