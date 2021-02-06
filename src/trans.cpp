#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"

int main(int argc, char **argv) {
	Driver transDriver {};
	transDriver.run(ConfigurationParser {argc, argv} );
	return 0;
}

