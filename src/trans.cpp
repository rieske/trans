#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"

#include <iostream>

int main(int argc, char **argv) {
	Driver transDriver {};
	return transDriver.run(ConfigurationParser {argc, argv});
}

