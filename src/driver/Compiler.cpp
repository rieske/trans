#include "Compiler.h"
#include "HostToolchain.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "CompilerComponentsFactory.h"
#include "scanner/TypedefRegistry.h"
#include "scanner/EnumConstantRegistry.h"
#include "scanner/FiniteAutomaton.h"

#include "types/Type.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/QuadrupleGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "semantic_analyzer/ValueEntry.h"
#include "codegen/quadruples/Quadruple.h"
#include "util/Logger.h"
#include "util/LogManager.h"

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

int runCommand(const std::string& command) {
    return system(command.c_str());
}

codegen::GlobalVariable toCodegenGlobal(const semantic_analyzer::ValueEntry& global) {
    codegen::GlobalVariable gv;
    gv.name = global.getName();
    gv.sizeInBytes = global.getType().getSize();
    gv.isExternal = global.isExternal();
    gv.isStatic = global.isStaticStorage();
    if (global.getType().isPrimitive() && global.getType().getPrimitive().isFloating()) {
        gv.valueType = codegen::Type::FLOATING;
    }
    if (global.getType().isPrimitive() && !global.getType().getPrimitive().isFloating()) {
        gv.isSigned = global.getType().getPrimitive().isSigned();
    }
    if (global.hasStringInitializer()) {
        gv.stringInitializer = global.getStringInitializer();
        gv.initializerLiteral = "0";
    } else if (global.hasMultiWordInitializer()) {
        gv.multiWordInitializer = global.getMultiWordInitializer();
        gv.initializerLiteral = "0";
    } else if (global.hasAddressInitializer()) {
        // NASM: p dq g  (address of another global)
        gv.initializerLiteral = global.getAddressInitializer();
    } else {
        // Hex for values outside signed 32-bit (includes IEEE double bit patterns).
        auto bits = static_cast<unsigned long long>(global.getConstantInitializer().value_or(0));
        if (bits > 0x7fffffffull) {
            std::ostringstream hex;
            hex << "0x" << std::hex << bits;
            gv.initializerLiteral = hex.str();
        } else {
            gv.initializerLiteral = std::to_string(static_cast<long long>(bits));
        }
    }
    return gv;
}

} // namespace

int assemble(std::string assemblyFileName, std::string objectFileName) {
    std::string assemblerCommand { "nasm -O1 -f elf64 -o " + shellQuote(objectFileName) + " " + shellQuote(assemblyFileName) };
    return runCommand(assemblerCommand);
}

int link(const std::vector<std::string>& objectFiles, const std::string& outputFile,
        const std::string& resourcesBasePath) {
    // NASM objects use absolute/PC32 relocs; default PIE link fails against libc.
    std::string linkerCommand = buildHostLinkCommand(objectFiles, outputFile, resourcesBasePath);
    if (linkerCommand.empty()) {
        err << "Linking failed: cannot find va_tls.o (build runtime/va_tls.c)\n";
        return 1;
    }
    return runCommand(linkerCommand);
}

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
    std::string command = configuration.getPreprocessor() + " " + hostGccPreprocessDialectFlags();
    for (const auto& includePath : configuration.getIncludePaths()) {
        command += " -I" + shellQuote(includePath);
    }
    for (const auto& define : configuration.getDefines()) {
        command += " -D" + shellQuote(define);
    }
    for (const auto& undef : configuration.getUndefines()) {
        command += " -U" + shellQuote(undef);
    }
    command += " " + hostGccPreprocessTrailingFlags();
    command += " -o " + shellQuote(preprocessedFile) + " " + shellQuote(sourceFileName);

    out << "Preprocessing " << sourceFileName << "...\n";
    if (runCommand(command) != 0) {
        throw std::runtime_error { "preprocessing failed for " + sourceFileName };
    }
    return preprocessedFile;
}

void Compiler::compileTranslationUnit(const std::string& sourceFileName, const std::string& assemblyFileName) const {
    out << "Compiling " << sourceFileName << "...\n";

    std::unique_ptr<scanner::Scanner> scanner = compilerComponentsFactory.makeScannerForSourceFile(sourceFileName);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder = compilerComponentsFactory.makeSyntaxTreeBuilder(sourceFileName, &grammar);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*syntaxTree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& global : semanticAnalyzer.getGlobalVariables()) {
        globalVariables.push_back(toCodegenGlobal(global));
    }

    codegen::QuadrupleGenerator quadrupleGenerator;
    std::vector<std::unique_ptr<codegen::Quadruple>> quadruples = quadrupleGenerator.generateQuadruplesFrom(*syntaxTree);

    if (configuration.isOutputIntermediateForms()) {
        out << "\nsymbol table\n";
        semanticAnalyzer.printSymbolTable();
        out << "symbol table end\n";
        out << "\nquadruples\n";
        for (auto &quadruple : quadruples) {
            out << *quadruple;
        }
        out << "quadruples end\n\n";
    }

    std::ofstream assemblyFile { assemblyFileName };
    if (!assemblyFile) {
        throw std::runtime_error { "could not open assembly output: " + assemblyFileName };
    }
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator = compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(std::move(quadruples), semanticAnalyzer.getConstants(), globalVariables);
    assemblyFile.close();
}

void Compiler::compile(std::string sourceFileName) const {
    // Fresh typedef / enum-constant tables per translation unit.
    scanner::TypedefRegistry::clear();
    scanner::EnumConstantRegistry::clear();
    scanner::FiniteAutomaton::clearTypedefNames();

    // First-class SysV AMD64 __builtin_va_list (array-of-1 of the 24-byte tag).
    // Headers may typedef va_list to this name after gcc -E.
    {
        type::Type vaList = type::builtinVaListType();
        scanner::TypedefRegistry::add("__builtin_va_list", vaList);
        scanner::FiniteAutomaton::registerTypedefName("__builtin_va_list");
    }

    std::string inputForCompiler = sourceFileName;
    bool removePreprocessed = false;

    if (!configuration.shouldSkipPreprocess() && needsPreprocessing(sourceFileName)) {
        inputForCompiler = preprocess(sourceFileName);
        removePreprocessed = true;
    }

    if (configuration.isPreprocessOnly()) {
        if (!configuration.getOutputFile().empty() && configuration.getOutputFile() != inputForCompiler) {
            std::string cmd = "cp " + shellQuote(inputForCompiler) + " " + shellQuote(configuration.getOutputFile());
            if (runCommand(cmd) != 0) {
                throw std::runtime_error { "failed to write preprocessed output" };
            }
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

    if (assemble(assemblyFileName, objectFileName) != 0) {
        throw std::runtime_error("Assembly failed for " + assemblyFileName);
    }
    // make CC=trans must not continue with a missing/empty .o (former trans-cc guard).
    ensureNonEmptyObjectFile(objectFileName);

    if (configuration.isCompileOnly()) {
        out << "Wrote object " << objectFileName << "\n";
        return;
    }

    std::string executableName = configuration.getOutputFile().empty()
            ? sourceFileName + ".out"
            : configuration.getOutputFile();

    if (link({ objectFileName }, executableName, configuration.getResourcesBasePath()) == 0) {
        out << "Successfully compiled and linked\n";
    } else {
        err << "Linking failed\n";
    }
}
