#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <memory>

class Configuration;

class Driver {
public:
	void run(std::unique_ptr<Configuration> configuration) const;
};

#endif /* TRANSDRIVER_H_ */
