#ifndef SEMANTIC_DECLARATIONANALYZER_H_
#define SEMANTIC_DECLARATIONANALYZER_H_

#include <functional>
#include <string>

#include "SymbolTable.h"
#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Declaration.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

namespace semantic_analyzer {


// Diagnostics callbacks so analyzer does not inherit the whole visitor.
struct SemanticDiagnostics {
    std::function<void(std::string message, const translation_unit::Context& context)> error;
};

// Compatible function types for redeclaration / definition vs prototype.
// Top-level const/volatile on parameters is ignored (C 6.7.6.3).
// Empty-vararg (0 fixed args) may be refined by a more specific prototype.
bool functionTypesCompatible(const type::Function& existing, const type::Function& incoming);

class DeclarationAnalyzer {
public:
    DeclarationAnalyzer(SymbolTable& symbols,
            SemanticDiagnostics diagnostics,
            ast::AbstractSyntaxTreeVisitor& childVisitor);

    void analyze(ast::Declaration& declaration);

private:
    void analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
            const type::Type& baseType,
            bool isExtern,
            bool isStaticStorage);

    SymbolTable& symbolTable;
    SemanticDiagnostics diagnostics;
    ast::AbstractSyntaxTreeVisitor& childVisitor;
};

} // namespace semantic_analyzer

#endif // SEMANTIC_DECLARATIONANALYZER_H_
