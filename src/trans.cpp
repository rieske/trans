#include "driver/TransCompiler.h"
#include "driver/TransDriver.h"
#include "driver/ConfigurationParser.h"
#include "parser/LR1Parser.h"

#include <string>

int main(int argc, char **argv) {
	ConfigurationParser configuration(argc, argv);
	Parser *parser = NULL;
	std::string customGrammarFileName = configuration.getCustomGrammarFileName();
	if (!customGrammarFileName.empty()) {
		parser = new LR1Parser(customGrammarFileName);
	} else {
		parser = new LR1Parser();
	}
	TransCompiler compiler(*parser);
	TransDriver transDriver(configuration, compiler);
	transDriver.run();
	delete parser;
	return 0;
}
