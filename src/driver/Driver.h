#ifndef TRANSDRIVER_H_
#define TRANSDRIVER_H_

#include <memory>

class Configuration;

class Driver {
public:
	Driver(const Configuration* configuration);
	virtual ~Driver();

	void run() const;

private:
	std::unique_ptr<const Configuration> configuration;
};

#endif /* TRANSDRIVER_H_ */
