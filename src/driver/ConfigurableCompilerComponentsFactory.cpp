#include "ConfigurableCompilerComponentsFactory.h"
#include "scanner/FiniteAutomatonScanner.h"
#include "scanner/FiniteAutomatonFactory.h"

using std::string;
using std::unique_ptr;

const string defaultScannerConfigurationFileName = "resources/configuration/scanner.lex";

ConfigurableCompilerComponentsFactory::ConfigurableCompilerComponentsFactory(const Configuration& configuration):
configuration(configuration) {
}

ConfigurableCompilerComponentsFactory::~ConfigurableCompilerComponentsFactory() {
}

unique_ptr<Scanner> ConfigurableCompilerComponentsFactory::getScanner() const {
	if (configuration.isScannerLoggingEnabled()) { // FIXME
		FiniteAutomatonScanner::set_logging("scanner.log");
	}
	unique_ptr<FiniteAutomatonFactory> finiteAutomatonFactory{new FiniteAutomatonFactory(defaultScannerConfigurationFileName)};
	unique_ptr<Scanner> scanner{new FiniteAutomatonScanner{std::move(finiteAutomatonFactory)}};
	return scanner;
}

