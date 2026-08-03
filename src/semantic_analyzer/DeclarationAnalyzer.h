#ifndef SEMANTIC_DECLARATIONANALYZER_H_
#define SEMANTIC_DECLARATIONANALYZER_H_

#include <functional>
#include <string>

#include "SymbolTable.h"
#include "ast/Declaration.h"
#include "translation_unit/Context.h"
#include "types/Type.h"
#include "symbols/AnnotationStore.h"

namespace ast {
class AbstractSyntaxTreeVisitor;
}

namespace semantic_analyzer {

// Diagnostics callbacks so analyzer does not inherit the whole visitor.
struct SemanticDiagnostics {
    std::function<void(std::string message, const translation_unit::Context& context)> error;
};

// Compatible function types for redeclaration / definition vs prototype.
// Top-level const/volatile on parameters is ignored (C 6.7.6.3).
// Empty-vararg (0 fixed args) may be refined by a more specific prototype.
bool functionTypesCompatible(const type::Function& existing, const type::Function& incoming);

enum class FunctionDeclareKind { Prototype, Definition };

// Single path for prototype and definition. On failure, writes a diagnostic
// into error and returns false. Definitions are marked defined on success.
bool declareFunction(SymbolTable& symbols,
        const std::string& name,
        const type::Function& type,
        const translation_unit::Context& context,
        bool internalLinkage,
        FunctionDeclareKind kind,
        std::string& error);

class DeclarationAnalyzer {
public:
    // visitor is used only for AST accept() on specs/declarators/initializers.
    DeclarationAnalyzer(SymbolTable& symbols,
            SemanticDiagnostics diagnostics,
            ast::AbstractSyntaxTreeVisitor& visitor,
            symbols::AnnotationStore& store);

    void analyze(ast::Declaration& declaration);

private:
    void analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
            const type::Type& baseType,
            bool isExtern,
            bool isStatic);
    // File-scope / static .data from an already-peeled effective initializer.
    void applyGlobalInitializer(ast::InitializedDeclarator& declarator,
            type::Type& type,
            ast::Expression* initAst,
            ast::Expression* effectiveInit);
    // Local aggregate brace/string -> FieldInit plan.
    void applyLocalAggregateInitializer(ast::InitializedDeclarator& declarator,
            ast::Expression* initAst);

    SymbolTable& symbolTable;
    SemanticDiagnostics diagnostics;
    ast::AbstractSyntaxTreeVisitor& visitor;
    symbols::AnnotationStore& store;
};

} // namespace semantic_analyzer

#endif // SEMANTIC_DECLARATIONANALYZER_H_
