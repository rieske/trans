#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <vector>

enum class AssemblyDialect {
    Intel,
    AtAndT
};

// Canonical short name for CLI and artifacts: "intel" | "att".
std::string assemblyDialectTag(AssemblyDialect dialect);

// How far the driver runs. Most-restrictive stage wins when multiple are set
// (rank is explicit in Configuration::setStopAfter, not enum storage order).
enum class StopAfter {
    Link,
    Object,
    Assembly,
    Preprocess
};

class Configuration {
  public:
    Configuration() = default;
    ~Configuration() = default;

    void setSourceFiles(std::vector<std::string> sourceFiles);
    void setResourcesBasePath(std::string resourcesBasePath);
    void setGrammarPath(std::string grammarPath);
    void setAssemblyDialect(AssemblyDialect dialect);
    void enableScannerLogging();
    void enableParserLogging();
    void setStopAfter(StopAfter stage);
    void setCompileOnly();
    void setAssemblyOnly();
    void setSaveTemps(bool saveTemps = true);
    void setOutputPath(std::string outputPath);
    void setGnuExtensions(bool enabled);
    void setPreprocessorStdFlag(std::string stdName);
    void setPreprocessorArgs(std::vector<std::string> args);
    void setPreprocessOnly();
    void setVerbose(bool verbose = true);
    void setIgnoredFlags(std::vector<std::string> flags);

    std::vector<std::string> getSourceFiles() const;
    std::string getLexPath() const;
    std::string getGrammarPath() const;
    std::string getParsingTablePath() const;
    AssemblyDialect getAssemblyDialect() const;
    std::string assemblyDialectTag() const;
    bool usingCustomGrammar() const;
    bool isScannerLoggingEnabled() const;
    bool isParserLoggingEnabled() const;
    StopAfter stopAfter() const;
    bool stopsBeforeLink() const;
    bool isCompileOnly() const;
    bool isAssemblyOnly() const;
    bool isSaveTemps() const;
    std::string getOutputPath() const;
    bool gnuExtensions() const;
    std::string getPreprocessorStdFlag() const;
    const std::vector<std::string>& getPreprocessorArgs() const;
    bool isPreprocessOnly() const;
    bool isVerbose() const;
    const std::vector<std::string>& getIgnoredFlags() const;

  private:
    std::vector<std::string> sourceFiles;
    std::string resourcesBasePath {};
    std::string lexPath {"resources/configuration/scanner.lex"};
    std::string grammarPath {"resources/configuration/grammar.bnf"};
    std::string parsingTablePath {"resources/configuration/parsing_table"};
    AssemblyDialect assemblyDialect { AssemblyDialect::Intel };
    bool customGrammar {false};
    bool scannerLogging {false};
    bool parserLogging {false};
    StopAfter stopAfter_ { StopAfter::Link };
    bool saveTemps_ {false};
    bool gnuExtensions_ {true};
    std::string preprocessorStdFlag_ {};
    std::vector<std::string> preprocessorArgs_ {};
    bool verbose_ {false};
    std::vector<std::string> ignoredFlags_ {};
    std::string outputPath {};
};

#endif // CONFIGURATION_H_
