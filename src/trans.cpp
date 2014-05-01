#include "driver/TransCompiler.h"
#include "driver/Driver.h"
#include "driver/ConfigurableCompilerComponentsFactory.h"
#include "driver/ConfigurationParser.h"
#include "parser/LR1Parser.h"

#include <string>

int main(int argc, char **argv) {
	ConfigurationParser configuration(argc, argv);
	ConfigurableCompilerComponentsFactory compilerComponentsFactory(configuration);
	Driver transDriver(configuration, compilerComponentsFactory);
	transDriver.run();
	return 0;
}
