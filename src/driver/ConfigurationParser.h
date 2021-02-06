#ifndef TRANSCONFIGURATION_H_
#define TRANSCONFIGURATION_H_

#include "driver/Configuration.h"
#include <string>
#include <vector>


class ConfigurationParser {
  public:
    ConfigurationParser(int argc, char **argv);
    ~ConfigurationParser();

    Configuration parseConfiguration();

    std::vector<std::string> getSourceFileNames() const;

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

    std::string resourcesBaseDir {};
    std::string grammarFileName;
    bool scannerLoggingEnabled {false};
    bool parserLoggingEnabled {false};
    bool customGrammarSet {false};
};

#endif // TRANSCONFIGURATION_H_
