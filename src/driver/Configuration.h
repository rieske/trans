#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <vector>

class Configuration {
  public:
    Configuration() = default;
    ~Configuration() = default;

    void setSourceFiles(std::vector<std::string> sourceFiles);
    void setResourcesBasePath(std::string resourcesBasePath);
    void setGrammarPath(std::string grammarPath);
    void enableParserLogging();
    void enableScannerLogging();
    void setOutputIntermediateForms();

    std::vector<std::string> getSourceFiles() const;
    std::string getLexPath() const;
    std::string getGrammarPath() const;
    std::string getParsingTablePath() const;
    bool usingCustomGrammar() const;
    bool isParserLoggingEnabled() const;
    bool isScannerLoggingEnabled() const;
    bool isOutputIntermediateForms() const;

  private:
    std::vector<std::string> sourceFiles;
    std::string resourcesBasePath {};
    std::string lexPath {"resources/configuration/scanner.lex"};
    std::string grammarPath {"resources/configuration/grammar.bnf"};
    std::string parsingTablePath {"resources/configuration/parsing_table"};
    bool customGrammar {false};
    bool parserLogging {false};
    bool scannerLogging {false};
    bool outputIntermediateForms {false};
};

#endif // CONFIGURATION_H_
