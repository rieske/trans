#include "CompileToIr.h"

#include "ResourceHelpers.h"
#include "ast/AbstractSyntaxTree.h"
#include "codegen/Instruction.h"
#include "codegen/IrGenerator.h"
#include "driver/CompilerComponentsFactory.h"
#include "driver/Configuration.h"
#include "driver/LanguageFrontEnd.h"
#include "parser/LR1Parser.h"
#include "scanner/LexicalSession.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "util/Diagnostic.h"

#include <cstring>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// $t / __L / L$str / L$cl counters are process-wide in the compiler.
// Remap each prefix to first-seen ordinals so dumps are order-stable.
std::string normalizeIrDump(const std::string& dump) {
    static const char* prefixes[] = { "L$str", "L$cl", "__L", "$t" };
    std::map<std::string, std::string> mapped;
    std::map<std::string, int> next;
    std::string out;
    out.reserve(dump.size());
    for (std::size_t i = 0; i < dump.size();) {
        bool hit = false;
        for (const char* prefix : prefixes) {
            const std::size_t n = std::strlen(prefix);
            if (dump.compare(i, n, prefix) == 0 && i + n < dump.size() && isDigit(dump[i + n])) {
                std::size_t j = i + n;
                while (j < dump.size() && isDigit(dump[j])) {
                    ++j;
                }
                const std::string raw = dump.substr(i, j - i);
                auto it = mapped.find(raw);
                if (it == mapped.end()) {
                    it = mapped.emplace(raw, std::string(prefix) + std::to_string(next[prefix]++)).first;
                }
                out += it->second;
                i = j;
                hit = true;
                break;
            }
        }
        if (!hit) {
            out += dump[i];
            ++i;
        }
    }
    return out;
}

} // namespace

std::string compileToIr(const std::string& source) {
    static int seq = 0;
    const std::string path = writeTempSource("compile_to_ir_" + std::to_string(seq++), source);

    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    CompilerComponentsFactory factory { configuration };
    const auto frontEnd = factory.makeFrontEnd();
    parser::LR1Parser parser { frontEnd->table() };

    scanner::LexicalSession session;
    auto scanner = factory.makeScannerForSourceFile(path, session);
    auto builder = factory.makeSyntaxTreeBuilder(&frontEnd->grammar(), session);
    auto syntaxTree = parser.parse(*scanner, *builder);
    auto* tree = dynamic_cast<ast::AbstractSyntaxTree*>(syntaxTree.get());
    if (!tree) {
        throw std::runtime_error { "compileToIr: expected AbstractSyntaxTree" };
    }

    semantic_analyzer::SemanticAnalyzer analyzer { configuration.gnuExtensions() };
    std::ostringstream ignored;
    diag::Sink sink { ignored };
    if (!analyzer.analyze(*tree, session, sink)) {
        throw std::runtime_error { "compileToIr: semantic errors\n" + ignored.str() };
    }

    codegen::IntermediateRepresentation ir = codegen::generateIr(*tree);
    for (const auto& procedure : ir.procedures) {
        for (const auto& instruction : procedure.body) {
            codegen::validateInstruction(instruction);
        }
        codegen::validateProcedureBody(procedure.body);
    }
    return normalizeIrDump(codegen::toString(ir));
}
