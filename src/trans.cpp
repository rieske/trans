#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"

int main(int argc, char **argv) {
	Driver transDriver {};
	transDriver.run(std::make_unique<ConfigurationParser>(argc, argv));
	return 0;
}

