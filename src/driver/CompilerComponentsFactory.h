#ifndef COMPILERCOMPONENTSFACTORY_H_
#define COMPILERCOMPONENTSFACTORY_H_

#include <memory>
#include <string>

class Configuration;
class Parser;
class Scanner;

class CompilerComponentsFactory {
public:
	CompilerComponentsFactory(const Configuration& configuration);
	virtual ~CompilerComponentsFactory();

	std::unique_ptr<Scanner> getScanner(std::string sourceFileName, std::string scannerConfigurationFileName =
			defaultScannerConfigurationFileName) const;
	std::unique_ptr<Parser> getParser() const;

private:
	const Configuration& configuration;

	static const std::string defaultScannerConfigurationFileName;
};

#endif /* COMPILERCOMPONENTSFACTORY_H_ */
