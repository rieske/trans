#ifndef CONFIGURABLECOMPILERCOMPONENTSFACTORY_H_
#define CONFIGURABLECOMPILERCOMPONENTSFACTORY_H_

#include "CompilerComponentsFactory.h"
#include "Configuration.h"

class ConfigurableCompilerComponentsFactory: public CompilerComponentsFactory {
public:
	ConfigurableCompilerComponentsFactory(const Configuration& configuration);
	virtual ~ConfigurableCompilerComponentsFactory();

	std::unique_ptr<Scanner> getScanner() const;

private:
	const Configuration& configuration;
};

#endif /* CONFIGURABLECOMPILERCOMPONENTSFACTORY_H_ */
