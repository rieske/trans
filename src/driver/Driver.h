#ifndef DRIVER_H_
#define DRIVER_H_

#include "driver/ConfigurationParser.h"

class Driver {
public:
	void run(ConfigurationParser configuration) const;
};

#endif // DRIVER_H_
