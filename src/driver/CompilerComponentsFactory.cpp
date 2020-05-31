#include "CompilerComponentsFactory.h"

#include "ast/AbstractSyntaxTreeBuilder.h"
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

using std::string;
using std::unique_ptr;

CompilerComponentsFactory::CompilerComponentsFactory(std::unique_ptr<Configuration> configuration) :
        configuration { std::move(configuration) }
{
}

unique_ptr<Scanner> CompilerComponentsFactory::makeScannerForSourceFile(std::string sourceFileName) const
{
    LexFileFiniteAutomaton* automaton { new LexFileFiniteAutomaton(configuration->getLexFileName()) };
    if (configuration->isScannerLoggingEnabled()) {
        Logger logger { &std::cout };
        logger << automaton;
    }
    return std::make_unique<FiniteAutomatonScanner>(new TranslationUnit { sourceFileName }, automaton);
}

unique_ptr<parser::Parser> CompilerComponentsFactory::makeParser() const {
    Logger logger { configuration->isParserLoggingEnabled() ? &std::cout : &nullStream };
    LogManager::registerComponentLogger(Component::PARSER, logger);

    parser::ParsingTable* parsingTable;
    if (configuration->usingCustomGrammar()) {
        parser::GeneratedParsingTable* generatedTable = new parser::GeneratedParsingTable(
                new parser::BNFFileGrammar(configuration->getGrammarFileName()), parser::LALR1Strategy { });
        if (configuration->isParserLoggingEnabled()) {
            generatedTable->persistToFile("logs/parsing_table");
            generatedTable->outputPretty("logs/parsing_table_pretty");
        }
        parsingTable = generatedTable;
    } else {
        parsingTable = new parser::FilePersistedParsingTable(configuration->getParsingTableFileName(),
                new parser::BNFFileGrammar(configuration->getGrammarFileName()));
    }

    return std::make_unique<parser::LR1Parser>(parsingTable);
}

unique_ptr<parser::SyntaxTreeBuilder> CompilerComponentsFactory::makeSyntaxTreeBuilder() const {
    return configuration->usingCustomGrammar() ?
                                                unique_ptr<parser::SyntaxTreeBuilder> { new parser::ParseTreeBuilder() } :
                                                unique_ptr<parser::SyntaxTreeBuilder> { new ast::AbstractSyntaxTreeBuilder() };
}

std::unique_ptr<codegen::AssemblyGenerator> CompilerComponentsFactory::makeAssemblyGenerator(std::ostream* assemblyFile) const {
    return std::make_unique<codegen::AssemblyGenerator>(
            std::make_unique<codegen::StackMachine>(
                    assemblyFile,
                    std::make_unique<codegen::IntelInstructionSet>(),
                    std::make_unique<codegen::Amd64Registers>()));
}
