#include "CompilerComponentsFactory.h"

#include "ast/AbstractSyntaxTreeBuilder.h"
#include "ast/GnuExtensions.h"
#include "parser/ParseExtensions.h"
#include "scanner/LexFileScannerReader.h"
#include "util/Logger.h"
#include "util/LogManager.h"

#include "codegen/ATandTInstructionSet.h"
#include "codegen/IntelInstructionSet.h"

#include <iostream>
#include <stdexcept>

CompilerComponentsFactory::CompilerComponentsFactory(Configuration configuration) :
        configuration { configuration }
{
}

std::unique_ptr<scanner::Scanner> CompilerComponentsFactory::makeScannerForSourceFile(
        std::string sourceFileName, scanner::LexicalSession& session) const {
    Logger logger { configuration.isScannerLoggingEnabled() ? &std::cout : &NullStream::getInstance() };
    LogManager::registerComponentLogger(Component::SCANNER, logger);

    scanner::LexFileScannerReader scannerReader;
    return std::make_unique<scanner::Scanner>(
            sourceFileName, scannerReader.fromConfiguration(configuration.getLexPath()), session);
}

std::shared_ptr<const LanguageFrontEnd> CompilerComponentsFactory::makeFrontEnd() const {
    Logger logger { configuration.isParserLoggingEnabled() ? &std::cout : &NullStream::getInstance() };
    LogManager::registerComponentLogger(Component::PARSER, logger);
    return LanguageFrontEnd::load(configuration);
}

std::unique_ptr<parser::SyntaxTreeBuilder> CompilerComponentsFactory::makeSyntaxTreeBuilder(
        const parser::Grammar* grammar, scanner::LexicalSession& session) const
{
    std::unique_ptr<parser::ParseExtensions> extensions;
    if (configuration.gnuExtensions()) {
        auto gnu = std::make_unique<ast::GnuExtensions>();
        gnu->installTypes(session);
        extensions = std::move(gnu);
    }
    return std::make_unique<ast::AbstractSyntaxTreeBuilder>(
            grammar, session, std::move(extensions), configuration.gnuExtensions());
}

std::unique_ptr<codegen::AssemblyGenerator> CompilerComponentsFactory::makeAssemblyGenerator(std::ostream* assemblyFile) const {
    std::unique_ptr<codegen::InstructionSet> instructionSet;
    switch (configuration.getAssemblyDialect()) {
    case AssemblyDialect::Intel:
        instructionSet = std::make_unique<codegen::IntelInstructionSet>();
        break;
    case AssemblyDialect::AtAndT:
        instructionSet = std::make_unique<codegen::ATandTInstructionSet>();
        break;
    default:
        throw std::logic_error { "unknown AssemblyDialect" };
    }
    return std::make_unique<codegen::AssemblyGenerator>(
            assemblyFile,
            std::move(instructionSet),
            std::make_unique<codegen::Amd64Registers>());
}
