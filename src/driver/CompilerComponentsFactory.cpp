#include "CompilerComponentsFactory.h"

#include "ast/AbstractSyntaxTreeBuilder.h"
#include "ast/VerboseSyntaxTreeBuilder.h"
#include "parser/BNFFileReader.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/GeneratedParsingTable.h"
#include "parser/LALR1Strategy.h"
#include "parser/LR1Parser.h"
#include "parser/ParseTreeBuilder.h"
#include "scanner/LexFileScannerReader.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "Configuration.h"

#include "codegen/AssemblyGenerator.h"
#include "codegen/IntelInstructionSet.h"

#include <chrono>

CompilerComponentsFactory::CompilerComponentsFactory(Configuration configuration) :
        configuration { configuration }
{
}

std::unique_ptr<scanner::Scanner> CompilerComponentsFactory::makeScannerForSourceFile(std::string sourceFileName) const {
    Logger logger { configuration.isScannerLoggingEnabled() ? &std::cout : &NullStream::getInstance() };
    LogManager::registerComponentLogger(Component::SCANNER, logger);

    scanner::LexFileScannerReader scannerReader;
    return std::make_unique<scanner::Scanner>(sourceFileName, scannerReader.fromConfiguration(configuration.getLexPath()));
}

parser::Grammar CompilerComponentsFactory::makeGrammar() const {
    parser::BNFFileReader reader;
    return reader.readGrammar(configuration.getGrammarPath());
}

std::unique_ptr<parser::Parser> CompilerComponentsFactory::makeParser(parser::Grammar* grammar) const {
    Logger logger { configuration.isParserLoggingEnabled() ? &std::cout : &NullStream::getInstance() };
    LogManager::registerComponentLogger(Component::PARSER, logger);

    parser::ParsingTable* parsingTable;
    if (configuration.usingCustomGrammar()) {
        parsingTable = generateParsingTable(grammar);
    } else {
        parsingTable = new parser::FilePersistedParsingTable(configuration.getParsingTablePath(), grammar);
    }

    return std::make_unique<parser::LR1Parser>(parsingTable);
}

parser::ParsingTable* CompilerComponentsFactory::generateParsingTable(const parser::Grammar* grammar) const {
    std::cout << "Generating parsing table" << std::endl;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    parser::GeneratedParsingTable* generatedTable = new parser::GeneratedParsingTable(grammar, parser::LALR1Strategy { });

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Parsing table generation took "
        << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
        << "[µs]" << std::endl;

    std::string parsingTableLocation {"logs/parsing_table"};
    generatedTable->persistToFile(parsingTableLocation);

    std::cout << "Parsing table saved to: " << parsingTableLocation << std::endl;

    return generatedTable;
}

std::unique_ptr<parser::SyntaxTreeBuilder> CompilerComponentsFactory::makeSyntaxTreeBuilder(
        std::string sourceFileName, const parser::Grammar* grammar) const
{
    if (configuration.usingCustomGrammar()) {
        return std::make_unique<parser::ParseTreeBuilder>(sourceFileName, grammar);
    }
    if (configuration.isSyntaxTreeLoggingEnabled()) {
        return std::make_unique<ast::VerboseSyntaxTreeBuilder>(sourceFileName, grammar);
    }
    return std::make_unique<ast::AbstractSyntaxTreeBuilder>(grammar);
}

std::unique_ptr<codegen::AssemblyGenerator> CompilerComponentsFactory::makeAssemblyGenerator(std::ostream* assemblyFile) const {
    return std::make_unique<codegen::AssemblyGenerator>(
            std::make_unique<codegen::StackMachine>(
                    assemblyFile,
                    std::make_unique<codegen::IntelInstructionSet>(),
                    std::make_unique<codegen::Amd64Registers>()));
}
