#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"

int main(int argc, char **argv) {
	Driver transDriver {};
	return transDriver.run(ConfigurationParser {argc, argv});
}
