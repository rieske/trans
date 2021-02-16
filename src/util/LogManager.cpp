#include "LogManager.h"

#include <stdexcept>

std::unique_ptr<LogManager> LogManager::instance;

LogManager::LogManager():
    outputLogger{&std::cout},
    errorLogger{&std::cerr}
{
}

LogManager::~LogManager() {
}

LogManager& LogManager::getInstance() {
	if (!instance) {
		instance = std::unique_ptr<LogManager>(new LogManager());
	}
	return *instance;
}

void LogManager::withOutputStreams(std::ostream& outputStream, std::ostream& errorStream, const std::function<void()>& action) {
	LogManager& logManager = LogManager::getInstance();
    logManager.outputLogger = Logger(&outputStream);
    logManager.errorLogger = Logger(&errorStream);

    action();

    logManager.outputLogger = Logger(&std::cout);
    logManager.errorLogger = Logger(&std::cerr);
}

Logger& LogManager::getOutputLogger() {
	LogManager& logManager = LogManager::getInstance();
    return logManager.outputLogger;
}

Logger& LogManager::getErrorLogger() {
	LogManager& logManager = LogManager::getInstance();
    return logManager.errorLogger;
}

Logger& LogManager::getComponentLogger(const Component component) {
	LogManager& logManager = LogManager::getInstance();
	try {
		return logManager.componentLoggers.at(component);
	} catch (std::out_of_range& notFound) {
		logManager.componentLoggers[component] = Logger { };
		return logManager.componentLoggers[component];
	}
}

void LogManager::registerComponentLogger(const Component component, Logger logger) {
	LogManager::getInstance().componentLoggers[component] = logger;
}
