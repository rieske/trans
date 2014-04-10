#include "TransConfiguration.h"

#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include <iterator>

static const char* const COMMAND_LINE_OPTIONS = "hl:g:";
static const char LOGGING_OPTION = 'l';
static const char GRAMMAR_OPTION = 'g';
static const char HELP_OPTION = 'h';
static const char SCANNER_LOGGING_FLAG = 's';
static const char PARSER_LOGGING_FLAG = 'p';

TransConfiguration::TransConfiguration(int argc, char **argv) :
		parserLoggingEnabled(false),
		scannerLoggingEnabled(false),
		customGrammarFilename("") {
	setExecutableName(argv);
	validateArguments(argc, argv);
	parseArgumentsVector(argc, argv);
}

TransConfiguration::~TransConfiguration() {
}

void TransConfiguration::parseArgumentsVector(int argc, char **argv) {
	int offset = parseOptions(argc, argv);
	parseSourceFileNames(argc - offset, argv + offset);
}

int TransConfiguration::parseOptions(int argc, char **argv) {
	opterr = 0;
	optind = 1;
	int option;
	while ((option = getopt(argc, argv, COMMAND_LINE_OPTIONS)) != -1) {
		switch (option) {
		break;
	case LOGGING_OPTION:
		setLogging(optarg);
		break;
	case GRAMMAR_OPTION:
		setGrammarFilename(optarg);
		break;
	case HELP_OPTION:
	default:
		printUsage();
		exit(EXIT_SUCCESS);
		}
	}
	return optind;
}

void TransConfiguration::parseSourceFileNames(int argc, char **argv) {
	for (int i = 0; i < argc; ++i) {
		sourceFileNames.push_back(argv[i]);
	}
	if (sourceFileNames.empty()) {
		outputErrorAndTerminate("No source files specified!");
	}
}

void TransConfiguration::setExecutableName(char **argv) {
	if (NULL != argv) {
		executableName = argv[0];
	}
}

void TransConfiguration::validateArguments(int argc, char **argv) const {
	if (argc <= 1 || argv == NULL) {
		printUsage();
		exit(EXIT_FAILURE);
	}
}

void TransConfiguration::setGrammarFilename(std::string parsedArgument) {
	customGrammarFilename = parsedArgument;
}

void TransConfiguration::setLogging(std::string loggingArguments) {
	for (std::string::iterator it = loggingArguments.begin(); it < loggingArguments.end(); it++) {
		switch (*it) {
		case SCANNER_LOGGING_FLAG:
			scannerLoggingEnabled = true;
			break;
		case PARSER_LOGGING_FLAG:
			parserLoggingEnabled = true;
			break;
		default:
			std::string errorMessage = "Invalid logging flag: ";
			errorMessage += *it;
			outputErrorAndTerminate(errorMessage);
		}
	}
}

void TransConfiguration::outputErrorAndTerminate(std::string errorMessage) const {
	std::cerr << errorMessage << std::endl;
	std::cerr << std::endl;
	printUsage();
	exit(EXIT_FAILURE);
}

void TransConfiguration::printUsage() const {
	std::cerr << "Usage: " << std::endl;
	std::cerr << executableName << " [options] source_file" << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << " -" << HELP_OPTION << "\t\tDisplay this information" << std::endl;
	std::cerr << " -" << LOGGING_OPTION << "<s|p>\tEnable scanner|parser logging" << std::endl;
	std::cerr << " -" << GRAMMAR_OPTION << "<file_name>\tSpecify custom grammar file" << std::endl;
}

const std::vector<std::string>& TransConfiguration::getSourceFileNames() const {
	return sourceFileNames;
}

const std::string TransConfiguration::getCustomGrammarFileName() const {
	return customGrammarFilename;
}

bool TransConfiguration::isParserLoggingEnabled() const {
	return parserLoggingEnabled;
}

bool TransConfiguration::isScannerLoggingEnabled() const {
	return scannerLoggingEnabled;
}
