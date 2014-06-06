#include <driver/ConfigurationParser.h>
#include <driver/Driver.h>

int main(int argc, char **argv) {
	Driver transDriver { new ConfigurationParser { argc, argv } };
	transDriver.run();
	return 0;
}
