#ifndef DRIVER_H_
#define DRIVER_H_

#include "driver/ConfigurationParser.h"

class Driver {
public:
	// Returns 0 if every source file compiled successfully, non-zero otherwise.
	int run(ConfigurationParser configuration) const;
};

#endif // DRIVER_H_
