#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <string>

#include "Configuration.h"
#include "Compiler.h"

class TransDriver {
public:
	TransDriver(const Configuration& configuration, const Compiler& compiler);
	virtual ~TransDriver();

	void run() const;

private:
	const Configuration& configuration;
	const Compiler& compiler;
};

#endif /* TRANSDRIVER_H_ */
