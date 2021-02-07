#include "ConfigurationParser.h"

#include <getopt.h>
#include <iostream>
#include <iterator>

static const char* const COMMAND_LINE_OPTIONS = "hl:g:r:";
static const char HELP_OPTION = 'h';
static const char LOGGING_OPTION = 'l';
static const char GRAMMAR_OPTION = 'g';
static const char RESOURCES_BASEDIR_OPTION = 'r';
static const char SCANNER_LOGGING_FLAG = 's';
static const char PARSER_LOGGING_FLAG = 'p';

ConfigurationParser::ConfigurationParser(int argc, char **argv) {
	setExecutableName(argv);
	validateArguments(argc, argv);
	parseArgumentsVector(argc, argv);
}

ConfigurationParser::~ConfigurationParser() {
}

Configuration ConfigurationParser::getConfiguration() const {
    return configuration;
}

void ConfigurationParser::parseArgumentsVector(int argc, char **argv) {
	int offset = parseOptions(argc, argv);
	parseSourceFileNames(argc - offset, argv + offset);
}

int ConfigurationParser::parseOptions(int argc, char **argv) {
	opterr = 0;
	optind = 1;
	int option;
	while ((option = getopt(argc, argv, COMMAND_LINE_OPTIONS)) != -1) {
		switch (option) {
            case LOGGING_OPTION:
                setLogging(optarg);
                break;
            case GRAMMAR_OPTION:
                configuration.setGrammarPath(optarg);
                break;
            case RESOURCES_BASEDIR_OPTION:
                configuration.setResourcesBasePath(optarg);
                break;
            case HELP_OPTION:
            default:
                printUsage();
                exit(EXIT_SUCCESS);
		}
	}
	return optind;
}

void ConfigurationParser::parseSourceFileNames(int argc, char **argv) {
    std::vector<std::string> sourceFilePaths;
	for (int i = 0; i < argc; ++i) {
		sourceFilePaths.push_back(argv[i]);
	}
    configuration.setSourceFiles(sourceFilePaths);
}

void ConfigurationParser::setExecutableName(char **argv) {
	if (NULL != argv) {
		executableName = argv[0];
	}
}

void ConfigurationParser::validateArguments(int argc, char **argv) const {
	if (argc <= 1 || argv == NULL) {
		printUsage();
		exit(EXIT_FAILURE);
	}
}

void ConfigurationParser::setLogging(std::string loggingArguments) {
	for (std::string::iterator it = loggingArguments.begin(); it < loggingArguments.end(); it++) {
		switch (*it) {
            case SCANNER_LOGGING_FLAG:
                configuration.enableScannerLogging();
                break;
            case PARSER_LOGGING_FLAG:
                configuration.enableParserLogging();
                break;
            default:
                std::string errorMessage = "Invalid logging flag: ";
                errorMessage += *it;
                outputErrorAndTerminate(errorMessage);
		}
	}
}

void ConfigurationParser::outputErrorAndTerminate(std::string errorMessage) const {
	std::cerr << errorMessage << std::endl;
	std::cerr << std::endl;
	printUsage();
	exit(EXIT_FAILURE);
}

void ConfigurationParser::printUsage() const {
	std::cerr << "Usage: " << std::endl;
	std::cerr << executableName << " [options] source_file" << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << " -" << HELP_OPTION << "\t\tDisplay this information" << std::endl;
	std::cerr << " -" << LOGGING_OPTION << "<s|p>\tEnable scanner|parser logging" << std::endl;
	std::cerr << " -" << GRAMMAR_OPTION << "<file_name>\tSpecify custom grammar file" << std::endl;
	std::cerr << " -" << RESOURCES_BASEDIR_OPTION << "<directory_path>\tSpecify custom resources base directory" << std::endl;
}

