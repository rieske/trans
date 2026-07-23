#include "SemanticAnalysisVisitorInternal.h"
#include "ArrayDecay.h"

#include "ast/FunctionDefinition.h"

#include <limits>
#include "Symbols.h"

namespace semantic_analyzer {




SemanticAnalysisVisitor::SemanticAnalysisVisitor() {
    // printf/scanf are variadic; fixed prototype is "int f()" with ..., so any arg count is allowed.
    type::Type functionType = type::function(type::signedInteger(), {}, true);
    symbolTable.insertFunction("printf", functionType.getFunction(), externalContext());
    symbolTable.insertFunction("scanf", functionType.getFunction(), externalContext());
    // Do not predeclare malloc: tests/headers may declare it with their own
    // prototype. Alloca lowering emits a direct call to the "malloc" symbol.
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
}

void SemanticAnalysisVisitor::printSymbolTable() const {
    symbolTable.printTable();
}

// Fold sizeof in array bounds without semanticError on missing symbols
// (used only from the late member-bound pass once file-scope symbols exist).
void SemanticAnalysisVisitor::foldSizeofInBound(ast::Expression* expr) {
    walkBoundExpressionTree(expr, [this](ast::UnaryExpression* unary) {
        ast::Expression* op = unary->getOperandExpression();
        if (auto* id = dynamic_cast<ast::IdentifierExpression*>(op)) {
            if (symbolTable.hasSymbol(id->getIdentifier())) {
                type::Type t = symbolTable.lookup(id->getIdentifier()).getType();
                unary->setSizeofValue(t.getSize());
            }
            return;
        }
        if (auto* access = dynamic_cast<ast::ArrayAccess*>(op)) {
            // sizeof(arr[0]) / sizeof((arr)[0]) - element size.
            ast::Expression* base = access->getLeftOperand();
            if (auto* id = dynamic_cast<ast::IdentifierExpression*>(base)) {
                if (symbolTable.hasSymbol(id->getIdentifier())) {
                    type::Type t = symbolTable.lookup(id->getIdentifier()).getType();
                    if (t.isArray()) {
                        unary->setSizeofValue(t.getElementType().getSize());
                    }
                }
            }
        }
    });
}

void SemanticAnalysisVisitor::applyPendingArrayMemberBounds() {
    if (!pendingArrayMembers || pendingArrayMembers->empty()) {
        return;
    }
    std::vector<ast::PendingArrayMemberGroup> remaining;
    remaining.reserve(pendingArrayMembers->groups().size());

    for (auto& group : pendingArrayMembers->groups()) {
        type::Type structType = group.structType;
        if (!structType.isRecord() || !structType.isCompleteRecord()) {
            remaining.push_back(std::move(group));
            continue;
        }

        std::map<std::string, type::Type> updates;
        bool canFold = true;
        for (auto& f : group.members) {
            if (!f.declarator) {
                continue;
            }
            f.declarator->foldArrayBoundSizeofs(
                    [this](ast::Expression* e) { foldSizeofInBound(e); });
            type::Type folded = f.declarator->getFundamentalType(f.baseType);
            // Array declarators that still decay to pointer mean the bound is not ready.
            // Pointer members (T *p) fold to pointer and must not block the group.
            if (f.declarator->hasArrayDeclarator() && folded.isPointer()) {
                canFold = false;
                break;
            }
            updates.insert_or_assign(f.memberName, folded);
        }
        if (!canFold) {
            remaining.push_back(std::move(group));
            continue;
        }

        std::vector<std::pair<std::string, type::Type>> members;
        members.reserve(structType.getStructMembers().size());
        for (const auto& m : structType.getStructMembers()) {
            auto it = updates.find(m.name);
            if (it != updates.end()) {
                members.emplace_back(m.name, it->second);
            } else if (m.type) {
                members.emplace_back(m.name, *m.type);
            }
        }
        if (structType.isUnion()) {
            type::completeUnion(structType, std::move(members));
        } else {
            type::completeStructure(structType, std::move(members));
        }
    }

    pendingArrayMembers->groups() = std::move(remaining);
}

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    if (declarationSpecifiers.getStorageSpecifiers().size() > 1) {
        semanticError("multiple storage classes in declaration specifiers",
                declarationSpecifiers.getStorageSpecifiers().at(1).getContext());
    }
    // Register enumerators as named integer constants.
    for (const auto& typeSpec : declarationSpecifiers.getTypeSpecifiers()) {
        for (const auto& enumerator : typeSpec.getEnumerators()) {
            if (!symbolTable.defineEnumConstant(enumerator.first, enumerator.second)) {
                semanticError("redefinition of enumerator `" + enumerator.first + "`", externalContext());
            }
        }
    }
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    SemanticDiagnostics diag;
    diag.error = [this](std::string msg, const translation_unit::Context& ctx) {
        semanticError(std::move(msg), ctx);
    };
    DeclarationAnalyzer{symbolTable, std::move(diag), *this}
            .analyze(declaration);
}

void SemanticAnalysisVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void SemanticAnalysisVisitor::visit(ast::InitializedDeclarator& declarator) {
    // Visit the declarator only. The initializer is analyzed from DeclarationAnalyzer
    // after the symbol is inserted (C 6.2.1: name is in scope for the initializer).
    declarator.getDeclarator()->accept(*this);
}

void SemanticAnalysisVisitor::visit(ast::Pointer&) {
}

void SemanticAnalysisVisitor::visit(ast::Identifier&) {
}

void SemanticAnalysisVisitor::visit(ast::ArrayDeclarator& declaration) {
    declaration.visitBaseDeclarator(*this);
    long size = 0;
    if (!declaration.subscriptExpression) {
        // Incomplete array T[] (params, flexible members) - size unused; type is array(0)
        // or pointer after FormalArgument adjustment.
        declaration.setArraySize(0);
        return;
    }
    // Fold integer constant expressions for the bound. Nested sizeof (git ARRAY_SIZE:
    // sizeof(x)/sizeof((x)[0])) must be visited so sizeofValue is set before re-folding;
    // otherwise the bound stays non-constant, getFundamentalType decays the local to a
    // pointer, and char seen[ARRAY_SIZE(key_val)] = {0} becomes a bogus char*.
    // Only walk sizeof nodes - full accept() would resolve VLA param bounds like
    // regmatch_t pmatch[__nmatch] in system headers before the name is in scope.
    // True VLAs (identifier bounds) still fail evaluateConstant and keep size 0.
    if (!declaration.subscriptExpression->evaluateConstant(size)) {
        walkBoundExpressionTree(declaration.subscriptExpression.get(),
                [this](ast::UnaryExpression* unary) { unary->accept(*this); });
        declaration.subscriptExpression->evaluateConstant(size);
    }
    if (size < 0) {
        semanticError("array size is negative", declaration.getContext());
        size = 0;
    } else if (size > static_cast<long>(std::numeric_limits<int>::max())) {
        // Bound must fit int elementCount for type::array; also avoids later overflow.
        semanticError("array size is too large", declaration.getContext());
        size = 0;
    }
    // size stays 0 for non-constant bounds (VLA parameter, etc.).
    declaration.setArraySize(size);
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitBaseDeclarator(*this);
    declarator.visitFormalArguments(*this);

    argumentNames.clear();
    for (auto& argumentDeclaration : declarator.getFormalArguments()) {
        argumentNames.push_back(argumentDeclaration.getName());
    }
    // Function registration is done by Declaration (prototypes) or FunctionDefinition,
    // using the full declarator type (so `int *f()` and `int (*f)()` are distinguished).
}

void SemanticAnalysisVisitor::visit(ast::FormalArgument& argument) {
    argument.visitSpecifiers(*this);
    argument.visitDeclarator(*this);
    // Abstract parameters (no name) appear in function pointer types; skip void-name check.
    if (argument.hasDeclarator() && argument.getType().isVoid()) {
        semanticError("function argument ‘" + argument.getName() + "’ declared void", argument.getDeclarationContext());
    }
}

void SemanticAnalysisVisitor::registerFunctionDefinition(ast::FunctionDefinition& function) {
    function.visitReturnType(*this);
    type::Type baseType = type::signedInteger();
    if (!function.getReturnTypeSpecifiers().getTypeSpecifiers().empty()) {
        baseType = function.getReturnTypeSpecifiers().getResolvedType();
    }
    function.visitDeclarator(*this);

    type::Type functionType = function.getDeclaratorType(baseType);
    if (!functionType.isFunction()) {
        semanticError("function definition declarator is not a function", function.getDeclaratorContext());
        return;
    }
    if (symbolTable.hasGlobalVariable(function.getName())) {
        semanticError("function `" + function.getName() + "` conflicts with global variable of the same name",
                function.getDeclaratorContext());
        return;
    }
    if (symbolTable.hasFunction(function.getName())) {
        auto existing = symbolTable.findFunction(function.getName());
        if (!functionTypesCompatible(existing.getType(), functionType.getFunction())) {
            semanticError("function `" + function.getName() + "` definition conflicts with previous one on "
                    + to_string(existing.getContext()), function.getDeclaratorContext());
            return;
        }
    } else {
        symbolTable.insertFunction(function.getName(), functionType.getFunction(), function.getDeclaratorContext());
    }

    function.setSymbol(symbolTable.findFunction(function.getName()));
    // Cache parameter names so phase 2 can start the frame without re-walking the declarator.
    function.setParameterNames(argumentNames);
}

void SemanticAnalysisVisitor::analyzeFunctionBody(ast::FunctionDefinition& function) {
    if (!function.hasSymbol()) {
        // Registration failed in phase 1; nothing further to do.
        return;
    }

    // Phase 1 already registered the function and recorded parameter names.
    symbolTable.startFunction(function.getName(), function.getParameterNames());
    namedLabels.clear();
    pendingGotos.clear();
    currentFunctionReturnType = function.getSymbol()->returnType();
    // Parameters and outermost body declarations share one scope (C); do not enterBlockScope.
    function.visitBodyChildren(*this);
    currentFunctionReturnType.reset();

    for (auto* gotoStmt : pendingGotos) {
        auto it = namedLabels.find(gotoStmt->getLabelName());
        if (it == namedLabels.end()) {
            semanticError("label `" + gotoStmt->getLabelName() + "` used but not defined",
                    gotoStmt->label.context);
        } else {
            gotoStmt->setTarget(it->second);
        }
    }
    namedLabels.clear();
    pendingGotos.clear();

    function.setArguments(symbolTable.getCurrentScopeArguments());
    function.setLocalVariables(symbolTable.getCurrentScopeSymbols());
    symbolTable.endFunction();
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
    registerFunctionDefinition(function);
    if (skipFunctionBodies) {
        return;
    }
    analyzeFunctionBody(function);
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    symbolTable.enterBlockScope();
    block.visitChildren(*this);
    symbolTable.exitBlockScope();
}

void SemanticAnalysisVisitor::requireProductAssignable(const type::Type& source, const type::Type& dest,
        const translation_unit::Context& context)
{
    if (!type::productCanAssignFrom(dest, source)) {
        semanticError("type mismatch: can't convert " + source.to_string() + " to " + dest.to_string(), context);
    }
}

void SemanticAnalysisVisitor::analyzeAsRvalue(ast::Expression& expr) {
    expr.accept(*this);
    decayArrayInPlace(expr, symbolTable);
}

void SemanticAnalysisVisitor::analyzeAsRvalue(ast::Expression* expr) {
    if (expr) {
        analyzeAsRvalue(*expr);
    }
}

void SemanticAnalysisVisitor::semanticError(std::string message, const translation_unit::Context& context) {
    containsSemanticErrors = true;
    semanticErrorLogger() << context << ": error: " << message << "\n";
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !containsSemanticErrors;
}

std::map<std::string, std::string> SemanticAnalysisVisitor::getConstants() const {
    return symbolTable.getConstants();
}

std::vector<ValueEntry> SemanticAnalysisVisitor::getGlobalVariables() const {
    return symbolTable.getGlobalVariables();
}


} // namespace semantic_analyzer
