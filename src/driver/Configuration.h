#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <vector>

// Assembly syntax / backend dialect (InstructionSet + assembler).
enum class AssemblyDialect {
    Intel,  // NASM Intel syntax (default)
    AtAndT  // GNU as AT&T syntax
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
    void enableSyntaxTreeLogging();
    void setOutputIntermediateForms();

    std::vector<std::string> getSourceFiles() const;
    std::string getLexPath() const;
    std::string getGrammarPath() const;
    std::string getParsingTablePath() const;
    AssemblyDialect getAssemblyDialect() const;
    // Short tag for artifact suffixes and CLI: "intel" | "att".
    std::string assemblyDialectTag() const;
    bool usingCustomGrammar() const;
    bool isScannerLoggingEnabled() const;
    bool isParserLoggingEnabled() const;
    bool isSyntaxTreeLoggingEnabled() const;
    bool isOutputIntermediateForms() const;

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
    bool syntaxTreeLogging {false};
    bool outputIntermediateForms {false};
};

#endif // CONFIGURATION_H_
