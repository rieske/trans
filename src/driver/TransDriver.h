#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <string>

#include "TransConfiguration.h"

class TransDriver {
public:
	TransDriver(TransConfiguration& transConfiguration);
	virtual ~TransDriver();

	void run() const;

private:
	void compile(const std::string& sourceFileName) const;

	TransConfiguration& transConfiguration;
};

#endif /* TRANSDRIVER_H_ */
