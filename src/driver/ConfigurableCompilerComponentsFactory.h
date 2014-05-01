#ifndef CONFIGURABLECOMPILERCOMPONENTSFACTORY_H_
#define CONFIGURABLECOMPILERCOMPONENTSFACTORY_H_

#include <memory>

#include "CompilerComponentsFactory.h"

class Configuration;

class ConfigurableCompilerComponentsFactory: public CompilerComponentsFactory {
public:
	ConfigurableCompilerComponentsFactory(const Configuration& configuration);
	virtual ~ConfigurableCompilerComponentsFactory();

	std::unique_ptr<Scanner> getScanner() const;
	std::unique_ptr<Compiler> getCompiler() const;

private:
	std::unique_ptr<Parser> getParser() const;

	const Configuration& configuration;
};

#endif /* CONFIGURABLECOMPILERCOMPONENTSFACTORY_H_ */
