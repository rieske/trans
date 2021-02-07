#ifndef CONFIGURATION_PARSER_H_
#define CONFIGURATION_PARSER_H_

#include "driver/Configuration.h"
#include <string>
#include <vector>


class ConfigurationParser {
  public:
    ConfigurationParser(int argc, char **argv);
    ~ConfigurationParser();

    Configuration getConfiguration() const;

  private:
    void setExecutableName(char **argv);
    void validateArguments(int argc, char **argv) const;
    void parseArgumentsVector(int argc, char **argv);
    int parseOptions(int argc, char **argv);
    void parseSourceFileNames(int argc, char **argv);

    void setLogging(std::string loggingArguments);

    void outputErrorAndTerminate(std::string errorMessage) const;
    void printUsage() const;

    std::string executableName;

    Configuration configuration;
};

#endif // CONFIGURATION_PARSER_H_
