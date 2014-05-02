#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <vector>

class Configuration {
public:
	Configuration() {
	}
	Configuration(const Configuration& that) = delete;

	virtual ~Configuration() {
	}

	virtual const std::vector<std::string> &getSourceFileNames() const = 0;
	virtual const std::string getCustomGrammarFileName() const = 0;
	virtual bool usingCustomGrammar() const = 0;
	virtual bool isParserLoggingEnabled() const = 0;
	virtual bool isScannerLoggingEnabled() const = 0;
};

#endif /* CONFIGURATION_H_ */
