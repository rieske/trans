#include "ConfigurableCompilerComponentsFactory.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include "../scanner/FiniteAutomatonFactory.h"
#include "../scanner/FiniteAutomatonScanner.h"
#include "../util/Logger.h"

using std::string;
using std::unique_ptr;

const string defaultScannerConfigurationFileName = "resources/configuration/scanner.lex";

ConfigurableCompilerComponentsFactory::ConfigurableCompilerComponentsFactory(const Configuration& configuration) :
		configuration(configuration) {
}

ConfigurableCompilerComponentsFactory::~ConfigurableCompilerComponentsFactory() {
}

unique_ptr<Scanner> ConfigurableCompilerComponentsFactory::getScanner() const {
	if (configuration.isScannerLoggingEnabled()) { // FIXME

	}
	unique_ptr<Logger> logger { new Logger {std::cout} };
	unique_ptr<FiniteAutomatonFactory> finiteAutomatonFactory { new FiniteAutomatonFactory(defaultScannerConfigurationFileName) };
	unique_ptr<Scanner> scanner { new FiniteAutomatonScanner { std::move(finiteAutomatonFactory) } };
	return scanner;
}

