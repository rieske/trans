#ifndef LOGMANAGER_H_
#define LOGMANAGER_H_

#include <map>
#include <memory>
#include <functional>

#include "Logger.h"

class Logger;

enum class Component {
	SCANNER, PARSER, SEMANTIC_ANALYZER, CODE_GENERATOR
};

class LogManager {
public:
	virtual ~LogManager();

    static void withErrorStream(std::ostream& errorStream, const std::function<void()>& action);

    static Logger& getErrorLogger();
	static Logger& getComponentLogger(const Component component);
	static void registerComponentLogger(const Component component, Logger logger);
private:
	LogManager();
	static LogManager& getInstance();

	static std::unique_ptr<LogManager> instance;

	std::map<Component, Logger> componentLoggers;
    Logger errorLogger;
};

#endif /* LOGMANAGER_H_ */
