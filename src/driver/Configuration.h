#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <vector>

class Configuration {
  public:
    Configuration() = default;
    Configuration(const Configuration &that) = delete;

    virtual ~Configuration() = default;

    virtual std::vector<std::string> getSourceFileNames() const = 0;
    virtual std::string getLexFileName() const = 0;
    virtual std::string getGrammarFileName() const = 0;
    virtual std::string getParsingTableFileName() const = 0;
    virtual bool usingCustomGrammar() const = 0;
    virtual bool isParserLoggingEnabled() const = 0;
    virtual bool isScannerLoggingEnabled() const = 0;
    virtual bool isOutputIntermediateForms() const = 0;
};

#endif /* CONFIGURATION_H_ */
