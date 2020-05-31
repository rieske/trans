#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"

int main(int argc, char **argv) {
	Driver transDriver { std::make_unique<ConfigurationParser>(argc, argv) };
	transDriver.run();
	return 0;
}

