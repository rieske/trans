#ifndef TRANSCONFIGURATION_H_
#define TRANSCONFIGURATION_H_

#include <string>
#include <vector>

class TransConfiguration {
public:
	TransConfiguration(int argc, char **argv);
	virtual ~TransConfiguration();

	const std::vector<std::string> &getSourceFileNames() const;
	const std::string getCustomGrammarFileName() const;
	bool isParserLoggingEnabled() const;
	bool isScannerLoggingEnabled() const;

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

	std::string customGrammarFilename;
	bool scannerLoggingEnabled;
	bool parserLoggingEnabled;
};

#endif // TRANSCONFIGURATION_H_
