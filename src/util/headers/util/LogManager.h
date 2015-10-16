#ifndef LOGMANAGER_H_
#define LOGMANAGER_H_

#include <map>
#include <memory>

#include "Logger.h"

class Logger;

enum class Component {
	SCANNER, PARSER, SEMANTIC_ANALYZER, CODE_GENERATOR
};

class LogManager {
public:
	virtual ~LogManager();

	static Logger& getComponentLogger(const Component component);
	static void registerComponentLogger(const Component component, Logger logger);
private:
	LogManager();
	static LogManager& getInstance();

	static std::unique_ptr<LogManager> instance;

	std::map<Component, Logger> componentLoggers;
};

#endif /* LOGMANAGER_H_ */
