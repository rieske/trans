#ifndef LOGMANAGER_H_
#define LOGMANAGER_H_

#include <map>
#include <functional>

#include "Logger.h"

enum class Component {
	SCANNER, PARSER
};

class LogManager {
public:
    static void withOutputStreamsForTesting(std::ostream& outputStream, std::ostream& errorStream, const std::function<void()>& action);

    static Logger& getOutputLogger();
    static Logger& getErrorLogger();
	static Logger& getComponentLogger(const Component component);
	static void registerComponentLogger(const Component component, Logger logger);

private:
	LogManager();
	static LogManager& getInstance();

	std::map<Component, Logger> componentLoggers;
    Logger outputLogger;
    Logger errorLogger;
};

#endif // LOGMANAGER_H_
