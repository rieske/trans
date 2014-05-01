#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <string>

#include "Configuration.h"
#include "Compiler.h"
#include "CompilerComponentsFactory.h"

class Driver {
public:
	Driver(const Configuration& configuration, const CompilerComponentsFactory& compilerComponentsFactory);
	virtual ~Driver();

	void run() const;

private:
	const Configuration& configuration;
	const CompilerComponentsFactory& compilerComponentsFactory;
};

#endif /* TRANSDRIVER_H_ */
