#include "SemanticAnalysisVisitorInternal.h"
#include "DeclarationAnalyzer.h"

#include "ast/ArrayDeclarator.h"
#include "ast/Declaration.h"
#include "ast/FunctionDefinition.h"
#include "builtins/BuiltinRegistry.h"
#include "builtins/BswapTable.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/InitializerListExpression.h"
#include "translation_unit/Context.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::applyPendingArrayMemberBounds() {
    if (pendingArrayMembers().empty()) {
        return;
    }
    std::vector<ast::PendingArrayMemberGroup> remaining;
    remaining.reserve(pendingArrayMembers().groups().size());

    for (auto& group : pendingArrayMembers().groups()) {
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
                    [this](ast::Expression* e) { foldSizeofInBound(e, symbolTable, *this); });
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

        std::vector<type::MemberSpec> members;
        members.reserve(structType.getStructMembers().size());
        for (const auto& m : structType.getStructMembers()) {
            std::optional<int> bitWidth;
            if (m.isBitField()) {
                bitWidth = m.bitField->width;
            }
            auto it = updates.find(m.name);
            if (it != updates.end()) {
                members.emplace_back(m.name, it->second, bitWidth);
            } else if (m.type) {
                members.emplace_back(m.name, *m.type, bitWidth);
            }
        }
        if (structType.isUnion()) {
            type::completeUnion(structType, std::move(members));
        } else {
            type::completeStructure(structType, std::move(members));
        }
    }

    pendingArrayMembers().groups() = std::move(remaining);
}

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    declarationSpecifiers.resolveTypeof(*this);
    if (declarationSpecifiers.getStorageSpecifiers().size() > 1) {
        semanticError("multiple storage classes in declaration specifiers",
                declarationSpecifiers.getStorageSpecifiers().at(1).getContext());
    }
    // Enumerator redefinition is diagnosed at parse time (ParseEnvironment::addEnumerator).
    // Import is solely SemanticAnalyzer's parseEnumConstants snapshot.
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    SemanticDiagnostics diag;
    diag.error = [this](std::string msg, const translation_unit::Context& ctx) {
        semanticError(std::move(msg), ctx);
    };
    DeclarationAnalyzer{symbolTable, std::move(diag), *this, annotations()}
            .analyze(declaration);
    if (symbolTable.isAtFileScope()) {
        applyPendingArrayMemberBounds();
    }
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
    if (!declaration.subscriptExpression) {
        // Incomplete T[] - getFundamentalType uses incompleteArray; do not treat as size 0.
        return;
    }
    // Fold ICE only. Named-declaration validation (negative / unfixed object)
    // lives on DeclarationAnalyzer / FormalArgument so type_name BUILD_ASSERT
    // char[-1] can visit this node and still clamp in getFundamentalType.
    // Only walk sizeof/offsetof/TypeCast/_Generic - full accept() would resolve
    // VLA param bounds like regmatch_t pmatch[__nmatch] before the name is in scope.
    // Do not setArraySize on Unfixed: that would hide parameter VLAs as size 0.
    long size = 0;
    if (!declaration.subscriptExpression->evaluateConstant(size)) {
        walkBoundExpressionTree(declaration.subscriptExpression.get(),
                [this](ast::Expression* node) { node->accept(*this); });
    }
    if (declaration.foldOwnBound() == ast::ArrayBoundFold::TooLarge) {
        semanticError("array size is too large", declaration.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitBaseDeclarator(*this);
    declarator.visitFormalArguments(*this);
    // Function registration is done by Declaration (prototypes) or FunctionDefinition,
    // using the full declarator type (so `int *f()` and `int (*f)()` are distinguished).
}

void SemanticAnalysisVisitor::visit(ast::FormalArgument& argument) {
    argument.visitSpecifiers(*this);
    argument.visitDeclarator(*this);
    if (argument.hasDeclarator()) {
        argument.getDeclarator()->forEachArrayDeclarator([&](ast::ArrayDeclarator& array) {
            if (!array.subscriptExpression || array.hasArraySize()) {
                return;
            }
            if (array.foldOwnBound() == ast::ArrayBoundFold::Negative) {
                semanticError("array size is negative", argument.getDeclarationContext());
            }
        });
    }
    // Abstract parameters (no name) appear in function pointer types; skip void-name check.
    if (argument.hasDeclarator() && argument.getType().isVoid()) {
        semanticError("function argument ‘" + argument.getName() + "’ declared void", argument.getDeclarationContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
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
    std::string error;
    if (!declareFunction(symbolTable, function.getName(), functionType.getFunction(),
            function.getDeclaratorContext(),
            function.getReturnTypeSpecifiers().hasStorage(ast::Storage::STATIC),
            FunctionDeclareKind::Definition, error)) {
        semanticError(std::move(error), function.getDeclaratorContext());
        return;
    }

    annotations().functionFrame(&function).symbol = std::make_unique<symbols::FunctionEntry>(
            symbolTable.findFunction(function.getName()));

    symbolTable.startFunction(function.getName(), function.parameterNames());
    namedLabels.clear();
    pendingGotos.clear();
    currentFunctionReturnType = annotations().functionFrame(&function).symbol.get()->returnType();
    currentFunctionName = function.getName();
    // Parameters and outermost body declarations share one scope (C); do not enterBlockScope.
    function.visitBodyChildren(*this);
    currentFunctionReturnType.reset();
    currentFunctionName.clear();

    for (auto* gotoStmt : pendingGotos) {
        auto it = namedLabels.find(gotoStmt->getLabelName());
        if (it == namedLabels.end()) {
            semanticError("label `" + gotoStmt->getLabelName() + "` used but not defined",
                    gotoStmt->label.context);
        } else {
            annotations().setLabel(gotoStmt, symbols::LabelSlot::Target, it->second);
        }
    }
    namedLabels.clear();
    pendingGotos.clear();

    annotations().functionFrame(&function).arguments = symbolTable.getCurrentScopeArguments();
    annotations().functionFrame(&function).locals = symbolTable.getCurrentScopeSymbols();
    symbolTable.endFunction();
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    symbolTable.enterBlockScope();
    block.visitChildren(*this);
    symbolTable.exitBlockScope();
}

void SemanticAnalysisVisitor::importParseEnumConstant(const std::string& name, long value) {
    if (!symbolTable.hasEnumConstant(name)) {
        symbolTable.defineEnumConstant(name, value);
    }
}

void SemanticAnalysisVisitor::requireProductAssignable(const type::Type& dest, const type::Type& source,
        const translation_unit::Context& context)
{
    if (!type::productAssignFrom(dest, source)) {
        semanticError(type::productAssignFailureMessage(dest, source), context);
    }
}

void SemanticAnalysisVisitor::requireValueCompatible(const type::Type& a, const type::Type& b,
        const translation_unit::Context& context)
{
    if (!type::productValueCompatible(a, b)) {
        semanticError("type mismatch: incompatible operands " + a.to_string() + " and " + b.to_string(), context);
    }
}

void SemanticAnalysisVisitor::requireArithmeticCompatible(const type::Type& a, const type::Type& b,
        const translation_unit::Context& context)
{
    if (!type::productArithmeticCompatible(a, b)) {
        semanticError("invalid operands to binary operator", context);
    }
}


void SemanticAnalysisVisitor::analyzeAsRvalue(ast::Expression& expr) {
    expr.accept(*this);
    decayArrayInPlace(expr, symbolTable, annotations());
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

std::vector<ValueEntry> SemanticAnalysisVisitor::getDataHomes() const {
    return symbolTable.getDataHomes();
}


// Inserts ordinary FunctionEntry symbols for designator / & uses.
// Calls still go through builtins::lookupBuiltin -> BuiltinPlan (see visit FunctionCall).
// Types come from lookupBuiltin so they stay aligned with BswapPlan descriptors.
void SemanticAnalysisVisitor::installGnuBuiltins() {
    if (!gnuExtensions_) {
        return;
    }
    const translation_unit::Context ctx { "<gnu>", 0 };
    for (const auto& builtin : builtins::kBswapBuiltins) {
        const auto desc = builtins::lookupBuiltin(builtin.name);
        if (!desc) {
            throw std::logic_error {
                    std::string("kBswapBuiltins entry missing from lookupBuiltin: ") + builtin.name
            };
        }
        type::Type value = desc->returnType;
        type::Type fn = type::function(value, { value });
        symbolTable.insertFunction(builtin.name, fn.getFunction(), ctx, false);
    }
}

} // namespace semantic_analyzer
