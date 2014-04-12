#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <string>

#include "Configuration.h"

class TransDriver {
public:
	TransDriver(Configuration& configuration);
	virtual ~TransDriver();

	void run() const;

private:
	void compile(const std::string& sourceFileName) const;

	Configuration& configuration;
};

#endif /* TRANSDRIVER_H_ */
