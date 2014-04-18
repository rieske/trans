#include "driver/TransCompiler.h"
#include "driver/Driver.h"
#include "driver/ConfigurableCompilerComponentsFactory.h"
#include "driver/ConfigurationParser.h"
#include "parser/LR1Parser.h"

#include <string>

int main(int argc, char **argv) {
	ConfigurationParser configuration(argc, argv);
	Parser *parser = nullptr;
	std::string customGrammarFileName = configuration.getCustomGrammarFileName();
	if (!customGrammarFileName.empty()) {
		parser = new LR1Parser(customGrammarFileName);
	} else {
		parser = new LR1Parser();
	}
	TransCompiler compiler(*parser);

	ConfigurableCompilerComponentsFactory compilerComponentsFactory(configuration);
	Driver transDriver(configuration, compiler, compilerComponentsFactory);
	transDriver.run();
	delete parser;
	return 0;
}
