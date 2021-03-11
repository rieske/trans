#include "CompilerComponentsFactory.h"

#include "ast/AbstractSyntaxTreeBuilder.h"
#include "ast/VerboseSyntaxTreeBuilder.h"
#include "parser/BNFFileGrammar.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/GeneratedParsingTable.h"
#include "parser/LALR1Strategy.h"
#include "parser/LR1Parser.h"
#include "parser/ParseTreeBuilder.h"
#include "scanner/FiniteAutomatonScanner.h"
#include "scanner/LexFileFiniteAutomaton.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "Configuration.h"
#include "translation_unit/TranslationUnit.h"

#include "codegen/AssemblyGenerator.h"
#include "codegen/IntelInstructionSet.h"

CompilerComponentsFactory::CompilerComponentsFactory(Configuration configuration) :
        configuration { configuration }
{
}

std::unique_ptr<scanner::Scanner> CompilerComponentsFactory::makeScannerForSourceFile(std::string sourceFileName) const
{
    scanner::LexFileFiniteAutomaton* automaton { new scanner::LexFileFiniteAutomaton(configuration.getLexPath()) };
    if (configuration.isScannerLoggingEnabled()) {
        Logger logger { &std::cout };
        logger << automaton;
    }
    return std::make_unique<scanner::FiniteAutomatonScanner>(new TranslationUnit { sourceFileName }, automaton);
}

std::unique_ptr<parser::Grammar> CompilerComponentsFactory::makeGrammar() const {
    return std::make_unique<parser::BNFFileGrammar>(configuration.getGrammarPath());
}

std::unique_ptr<parser::Parser> CompilerComponentsFactory::makeParser(parser::Grammar* grammar) const {
    Logger logger { configuration.isParserLoggingEnabled() ? &std::cout : &NullStream::getInstance() };
    LogManager::registerComponentLogger(Component::PARSER, logger);

    parser::ParsingTable* parsingTable;
    if (configuration.usingCustomGrammar()) {
        parser::GeneratedParsingTable* generatedTable = new parser::GeneratedParsingTable(grammar, parser::LALR1Strategy { });
        if (configuration.isParserLoggingEnabled()) {
            generatedTable->persistToFile("logs/parsing_table");
            generatedTable->outputPretty("logs/parsing_table_pretty");
        }
        parsingTable = generatedTable;
    } else {
        parsingTable = new parser::FilePersistedParsingTable(configuration.getParsingTablePath(), grammar);
    }

    return std::make_unique<parser::LR1Parser>(parsingTable);
}

std::unique_ptr<parser::SyntaxTreeBuilder> CompilerComponentsFactory::makeSyntaxTreeBuilder(
        std::string sourceFileName, parser::Grammar* grammar) const
{
    if (configuration.usingCustomGrammar()) {
        return std::make_unique<parser::ParseTreeBuilder>(sourceFileName);
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
