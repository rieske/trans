#ifndef TRANSCONFIGURATION_H_
#define TRANSCONFIGURATION_H_

#include <string>
#include <vector>

#include "Configuration.h"

class ConfigurationParser : public Configuration {
public:
	ConfigurationParser(int argc, char **argv);
	virtual ~ConfigurationParser();

	virtual const std::vector<std::string> &getSourceFileNames() const;
	virtual const std::string getCustomGrammarFileName() const;
	virtual bool usingCustomGrammar() const;
	virtual bool isParserLoggingEnabled() const;
	virtual bool isScannerLoggingEnabled() const;

	enum {
		PRINT_HELP = -1
	};
private:
	void setExecutableName(char **argv);
	void validateArguments(int argc, char **argv) const;
	void parseArgumentsVector(int argc, char **argv);
	int parseOptions(int argc, char **argv);
	void parseSourceFileNames(int argc, char **argv);

	void setGrammarFilename(std::string parsedArgument);
	void setLogging(std::string loggingArguments);

	void outputErrorAndTerminate(std::string errorMessage) const;
	void printUsage() const;

	std::string executableName;

	std::vector<std::string> sourceFileNames;

	std::string customGrammarFilename = "";
	bool scannerLoggingEnabled = false;
	bool parserLoggingEnabled = false;
};

#endif // TRANSCONFIGURATION_H_
