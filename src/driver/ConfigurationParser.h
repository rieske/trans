#ifndef TRANSCONFIGURATION_H_
#define TRANSCONFIGURATION_H_

#include <string>
#include <vector>

#include "Configuration.h"

class ConfigurationParser : public Configuration {
  public:
    ConfigurationParser(int argc, char **argv);
    virtual ~ConfigurationParser();

    std::vector<std::string> getSourceFileNames() const override;
    std::string getLexFileName() const override;
    std::string getGrammarFileName() const override;
    std::string getParsingTableFileName() const override;
    bool usingCustomGrammar() const override;
    bool isParserLoggingEnabled() const override;
    bool isScannerLoggingEnabled() const override;

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

    std::string grammarFileName = "resources/configuration/grammar.bnf";
    std::string parsingTableFileName = "resources/configuration/parsing_table";
    bool scannerLoggingEnabled = false;
    bool parserLoggingEnabled = false;
    bool customGrammarSet = false;
};

#endif // TRANSCONFIGURATION_H_
