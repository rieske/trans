#include "LogManager.h"

#include <stdexcept>

std::map<Component, Logger> LogManager::componentLoggers;

Logger& LogManager::getComponentLogger(const Component component) {
	try {
		return componentLoggers.at(component);
	} catch (std::out_of_range& notFound) {
		componentLoggers[component] = Logger { };
		return componentLoggers[component];
	}
}

void LogManager::registerComponentLogger(const Component component, Logger logger) {
	componentLoggers[component] = logger;
}
