#include "Compiler.h"
#include "HostToolchain.h"

#include <variant>
#include <type_traits>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "CompilerComponentsFactory.h"

#include "types/Type.h"
#include "types/TypeQuery.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/IrGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"
#include "scanner/LexicalSession.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "symbols/ValueEntry.h"
#include "symbols/AnnotationStore.h"
#include "util/ImmediateFormat.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"

static Logger& out = LogManager::getOutputLogger();
static Logger& err = LogManager::getErrorLogger();

namespace {

bool needsPreprocessing(const std::string& sourceFileName) {
    // Already-preprocessed intermediate files.
    if (sourceFileName.size() >= 2) {
        auto ext = sourceFileName.substr(sourceFileName.size() - 2);
        if (ext == ".i") {
            return false;
        }
    }
    return true;
}

codegen::GlobalVariable toCodegenGlobal(const symbols::ValueEntry& global) {
    codegen::GlobalVariable gv;
    gv.name = global.getName();
    gv.sizeInBytes = global.getType().getSize();
    gv.isExternal = global.isExternal();
    gv.isStatic = global.isStaticStorage();
    // kind()-aware: pointer-to-float still carries a floating payload under Type.
    if (type::isFloating(global.getType())) {
        gv.valueType = codegen::ValueKind::FLOATING;
    }
    if (type::isIntegral(global.getType())) {
        gv.isSigned = type::valueIsSigned(global.getType());
    }
    // Prefer closed GlobalInitializer variant (sole payload).
    if (const auto* init = global.globalInitializer()) {
        std::visit([&](const auto& arm) {
            using T = std::decay_t<decltype(arm)>;
            if constexpr (std::is_same_v<T, symbols::StringInit>) {
                gv.stringInitializer = arm.value;
                gv.initializerLiteral = "0";
            } else if constexpr (std::is_same_v<T, symbols::MultiWordInit>) {
                gv.multiWordInitializer = arm.words;
                gv.initializerLiteral = "0";
            } else if constexpr (std::is_same_v<T, symbols::AddressInit>) {
                // NASM: p dq g  (address of another global)
                gv.initializerLiteral = arm.symbolName;
            } else if constexpr (std::is_same_v<T, symbols::ConstantInit>) {
                gv.initializerLiteral = util::wordImmediate(
                        static_cast<unsigned long long>(arm.value));
            }
        }, *init);
    } else {
        gv.initializerLiteral = "0";
    }
    return gv;
}

void assemble(const std::string& assemblyFileName, const std::string& objectFileName,
        AssemblyDialect dialect) {
    switch (dialect) {
    case AssemblyDialect::Intel:
        util::runProcessOrThrow({
                "nasm", "-O1", "-f", "elf64",
                "-o", objectFileName,
                assemblyFileName
        });
        return;
    case AssemblyDialect::AtAndT:
        util::runProcessOrThrow({
                "as", "--64",
                "-o", objectFileName,
                assemblyFileName
        });
        return;
    }
    throw std::logic_error { "unknown AssemblyDialect" };
}

void link(const std::vector<std::string>& objectFiles, const std::string& outputFile) {
    // PIE link: codegen uses default rel / PLT / GOT for position-independent code.
    auto argv = buildHostLinkArgv(objectFiles, outputFile);
    util::runProcessOrThrow(argv);
}

} // namespace

Compiler::Compiler(Configuration configuration) :
        configuration { configuration },
        compilerComponentsFactory { configuration },
        grammar { compilerComponentsFactory.makeGrammar() },
        parser { compilerComponentsFactory.makeParser(&grammar) }
{
}

std::string Compiler::preprocess(const std::string& sourceFileName) const {
    std::string preprocessedFile = sourceFileName + ".i";
    // Dialect first; trailing defines after user -D/-U (must win). See HostToolchain.
    std::vector<std::string> argv { configuration.getPreprocessor() };
    for (const auto& flag : hostGccPreprocessDialectArgv()) {
        argv.push_back(flag);
    }
    for (const auto& includePath : configuration.getIncludePaths()) {
        argv.push_back("-I" + includePath);
    }
    for (const auto& define : configuration.getDefines()) {
        argv.push_back("-D" + define);
    }
    for (const auto& undef : configuration.getUndefines()) {
        argv.push_back("-U" + undef);
    }
    for (const auto& flag : hostGccPreprocessTrailingArgv()) {
        argv.push_back(flag);
    }
    argv.push_back("-o");
    argv.push_back(preprocessedFile);
    argv.push_back(sourceFileName);

    out << "Preprocessing " << sourceFileName << "...\n";
    util::runProcessOrThrow(argv);
    return preprocessedFile;
}

void Compiler::compileTranslationUnit(const std::string& sourceFileName, const std::string& assemblyFileName) const {
    out << "Compiling " << sourceFileName << "...\n";

    // Per-TU lexical state (typedefs, enums). Not process-static.
    scanner::LexicalSession session;
    {
        type::Type vaList = type::builtinVaListType();
        session.typedefs.add("__builtin_va_list", vaList);
    }

    std::unique_ptr<scanner::Scanner> scanner = compilerComponentsFactory.makeScannerForSourceFile(sourceFileName, session);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder =
            compilerComponentsFactory.makeSyntaxTreeBuilder(sourceFileName, &grammar, session);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*syntaxTree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& global : semanticAnalyzer.getGlobalVariables()) {
        globalVariables.push_back(toCodegenGlobal(global));
    }

    codegen::IntermediateRepresentation ir = codegen::generateIr(*syntaxTree);

    if (configuration.isOutputIntermediateForms()) {
        out << "\nsymbol table\n";
        semanticAnalyzer.printSymbolTable();
        out << "symbol table end\n";
        out << "\nir\n";
        out << ir;
        out << "ir end\n\n";
    }

    std::ofstream assemblyFile { assemblyFileName };
    if (!assemblyFile) {
        throw std::runtime_error { "could not open assembly output: " + assemblyFileName };
    }
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator = compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(ir, semanticAnalyzer.getConstants(), globalVariables);
    assemblyFile.close();
}

void Compiler::compile(std::string sourceFileName) const {
    std::string inputForCompiler = sourceFileName;
    bool removePreprocessed = false;

    if (!configuration.shouldSkipPreprocess() && needsPreprocessing(sourceFileName)) {
        inputForCompiler = preprocess(sourceFileName);
        removePreprocessed = true;
    }

    if (configuration.isPreprocessOnly()) {
        if (!configuration.getOutputFile().empty() && configuration.getOutputFile() != inputForCompiler) {
            util::runProcessOrThrow({ "cp", inputForCompiler, configuration.getOutputFile() });
        } else {
            // Default -E writes to stdout-like behavior: leave the .i file and report path.
            out << "Preprocessed output: " << inputForCompiler << "\n";
        }
        return;
    }

    // Assembly output path
    std::string assemblyFileName;
    if (configuration.isAssemblyOnly() && !configuration.getOutputFile().empty()) {
        assemblyFileName = configuration.getOutputFile();
    } else {
        assemblyFileName = sourceFileName + ".S";
    }

    compileTranslationUnit(inputForCompiler, assemblyFileName);

    if (removePreprocessed) {
        std::remove(inputForCompiler.c_str());
    }

    if (configuration.isAssemblyOnly()) {
        out << "Wrote assembly " << assemblyFileName << "\n";
        return;
    }

    // Object file path
    std::string objectFileName;
    if (configuration.isCompileOnly() && !configuration.getOutputFile().empty()) {
        objectFileName = configuration.getOutputFile();
    } else {
        objectFileName = sourceFileName + ".o";
    }

    assemble(assemblyFileName, objectFileName, configuration.getAssemblyDialect());
    // make CC=trans must not continue with a missing/empty .o (former trans-cc guard).
    ensureNonEmptyObjectFile(objectFileName);

    if (configuration.isCompileOnly()) {
        out << "Wrote object " << objectFileName << "\n";
        return;
    }

    std::string executableName = configuration.getOutputFile().empty()
            ? sourceFileName + ".out"
            : configuration.getOutputFile();

    try {
        link({ objectFileName }, executableName);
        out << "Successfully compiled and linked\n";
    } catch (const std::exception& ex) {
        err << "Linking failed: " << ex.what() << "\n";
        throw;
    }
}
