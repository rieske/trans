#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <memory>

class Configuration;

class Driver {
public:
	Driver(std::unique_ptr<Configuration> configuration);
	virtual ~Driver();

	void run() const;

private:
	std::unique_ptr<Configuration> configuration;
};

#endif /* TRANSDRIVER_H_ */
