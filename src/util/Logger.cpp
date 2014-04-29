#include "Logger.h"

Logger::Logger(std::ostream& outputStream = std::cout) :
		outputStream(outputStream) {
}

Logger::~Logger() {
}

template<typename T>
Logger& Logger::operator<<(const T& dataToLog) {
	outputStream << dataToLog;
	return *this;
}
