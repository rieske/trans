#include "driver/TransDriver.h"

int main(int argc, char **argv) {
	TransConfiguration transConfiguration(argc, argv);
	TransDriver transDriver(transConfiguration);
	transDriver.run();
	return 0;
}
