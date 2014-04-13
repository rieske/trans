#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <string>

#include "Configuration.h"
#include "Compiler.h"

class TransDriver {
public:
	TransDriver(Configuration& configuration, Compiler& compiler);
	virtual ~TransDriver();

	void run() const;

private:
	Configuration& configuration;
	Compiler& compiler;
};

#endif /* TRANSDRIVER_H_ */
