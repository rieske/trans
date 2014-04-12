#include "driver/TransDriver.h"
#include "driver/ConfigurationParser.h"

int main(int argc, char **argv) {
	ConfigurationParser configuration(argc, argv);
	TransDriver transDriver(configuration);
	transDriver.run();
	return 0;
}
