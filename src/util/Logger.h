#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>

class Logger {
public:
	Logger(std::ostream& outputStream);
	virtual ~Logger();

	template<typename T>
	Logger& operator<<(const T& dataToLog);

private:
	std::ostream& outputStream;
};

#endif /* LOGGER_H_ */
