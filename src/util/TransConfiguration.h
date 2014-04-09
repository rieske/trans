#ifndef _ARGV_PARSER_H_
#define _ARGV_PARSER_H_

#include <list>
#include <string>

class TransConfiguration {
public:
	TransConfiguration(int argc, char **argv);
	virtual ~TransConfiguration();

	const std::list<std::string> &getSourceFileNames() const;
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

	std::list<std::string> sourceFileNames;

	std::string customGrammarFilename;
	bool scannerLoggingEnabled;
	bool parserLoggingEnabled;

	static const char* const COMMAND_LINE_OPTIONS;
	static const char LOGGING_OPTION = 'l';
	static const char GRAMMAR_OPTION = 'g';
	static const char HELP_OPTION = 'h';
	static const char SCANNER_LOGGING_FLAG = 's';
	static const char PARSER_LOGGING_FLAG = 'p';
};

#endif // _ARGV_PARSER_H_
