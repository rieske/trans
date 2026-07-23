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
    void enableScannerLogging();
    void enableParserLogging();
    void enableSyntaxTreeLogging();
    void setOutputIntermediateForms();

    void addIncludePath(std::string path);
    void addDefine(std::string define);
    void addUndefine(std::string name);
    void setOutputFile(std::string outputFile);
    void setCompileOnly(bool compileOnly);
    void setPreprocessOnly(bool preprocessOnly);
    void setAssemblyOnly(bool assemblyOnly);
    void setSkipPreprocess(bool skip);
    void setPreprocessor(std::string preprocessor);
    void setObjectFiles(std::vector<std::string> objectFiles);
    void addLinkArg(std::string arg);
    void setLinkOnly(bool linkOnly);
    void addDepFile(std::string path);

    std::vector<std::string> getSourceFiles() const;
    std::string getLexPath() const;
    std::string getGrammarPath() const;
    std::string getParsingTablePath() const;
    const std::string& getResourcesBasePath() const;
    bool usingCustomGrammar() const;
    bool isScannerLoggingEnabled() const;
    bool isParserLoggingEnabled() const;
    bool isSyntaxTreeLoggingEnabled() const;
    bool isOutputIntermediateForms() const;

    const std::vector<std::string>& getIncludePaths() const;
    const std::vector<std::string>& getDefines() const;
    const std::vector<std::string>& getUndefines() const;
    const std::string& getOutputFile() const;
    bool isCompileOnly() const;
    bool isPreprocessOnly() const;
    bool isAssemblyOnly() const;
    bool shouldSkipPreprocess() const;
    const std::string& getPreprocessor() const;
    const std::vector<std::string>& getObjectFiles() const;
    const std::vector<std::string>& getLinkArgs() const;
    bool isLinkOnly() const;
    const std::vector<std::string>& getDepFiles() const;

  private:
    std::vector<std::string> sourceFiles;
    std::string resourcesBasePath {};
    std::string lexPath {"resources/configuration/scanner.lex"};
    std::string grammarPath {"resources/configuration/grammar.bnf"};
    std::string parsingTablePath {"resources/configuration/parsing_table"};
    bool customGrammar {false};
    bool scannerLogging {false};
    bool parserLogging {false};
    bool syntaxTreeLogging {false};
    bool outputIntermediateForms {false};

    std::vector<std::string> includePaths;
    std::vector<std::string> defines;
    std::vector<std::string> undefines;
    std::string outputFile {};
    bool compileOnly {false};
    bool preprocessOnly {false};
    bool assemblyOnly {false};
    bool skipPreprocess {false};
    std::string preprocessor {"gcc"};
    std::vector<std::string> objectFiles;
    std::vector<std::string> linkArgs;
    bool linkOnly {false};
    // Paths from -MF / -MFpath (make dependency files; written as stubs).
    std::vector<std::string> depFiles;
};

#endif // CONFIGURATION_H_
