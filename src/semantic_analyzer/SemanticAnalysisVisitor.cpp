#include "SemanticAnalysisVisitorInternal.h"
#include "DeclarationAnalyzer.h"

#include "ast/ArrayDeclarator.h"
#include "ast/Declaration.h"
#include "ast/FunctionDefinition.h"
#include "builtins/BuiltinRegistry.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializedDeclarator.h"
#include "ast/InitializerListExpression.h"
#include "translation_unit/Context.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    if (declarationSpecifiers.getStorageSpecifiers().size() > 1) {
        semanticError("multiple storage classes in declaration specifiers",
                declarationSpecifiers.getStorageSpecifiers().at(1).getContext());
    }
    for (auto& specifier : declarationSpecifiers.getTypeSpecifiers()) {
        finalizeSpecifierType(specifier);
    }
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    SemanticDiagnostics diag;
    diag.error = [this](std::string msg, const translation_unit::Context& ctx) {
        semanticError(std::move(msg), ctx);
    };
    DeclarationAnalyzer{symbolTable, std::move(diag), *this, annotations()}
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
    if (!declaration.subscriptExpression->foldToHostLong(size)) {
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
    try {
        auto type = argument.type();
        if (argument.hasDeclarator() && type.isVoid()) {
            semanticError("function argument ‘" + argument.getName() + "’ declared void",
                    argument.getDeclarationContext());
        }
    } catch (const std::invalid_argument& ex) {
        semanticError(ex.what(), argument.getDeclarationContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDefinition& function) {
    function.visitReturnType(*this);
    type::Type baseType = type::signedInteger();
    if (!function.getReturnTypeSpecifiers().getTypeSpecifiers().empty()) {
        baseType = function.getReturnTypeSpecifiers().getResolvedType();
    }
    function.visitDeclarator(*this);

    type::Type functionType { type::voidType() };
    try {
        functionType = function.getDeclaratorType(baseType);
    } catch (const std::invalid_argument& ex) {
        semanticError(ex.what(), function.getDeclaratorContext());
        return;
    }
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

    symbolTable.startFunction(function.getName(), function.definedFunctionParameterNames());
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

void SemanticAnalysisVisitor::importParseEnumConstant(const std::string& name,
        type::IntegerConstant value) {
    if (!symbolTable.hasEnumConstant(name)) {
        symbolTable.defineEnumConstant(name, std::move(value));
    }
}

bool SemanticAnalysisVisitor::checkProductAssign(const type::Type& dest, const type::Type& source,
        const translation_unit::Context& context, const ast::Expression* sourceExpr)
{
    return reportProductAssign(
            [this](std::string message, const translation_unit::Context& ctx) {
                semanticError(std::move(message), ctx);
            },
            dest, source, context, sourceExpr);
}

bool SemanticAnalysisVisitor::checkValueCompatible(const type::Type& a, const type::Type& b,
        const translation_unit::Context& context)
{
    if (type::productValueCompatible(a, b)) {
        return true;
    }
    semanticError("type mismatch: incompatible operands " + a.to_string() + " and " + b.to_string(), context);
    return false;
}

bool SemanticAnalysisVisitor::checkArithmeticCompatible(const type::Type& a, const type::Type& b,
        const translation_unit::Context& context)
{
    if (type::productArithmeticCompatible(a, b)) {
        return true;
    }
    semanticError("invalid operands to binary operator", context);
    return false;
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

void SemanticAnalysisVisitor::finalizeRecordDefinition(type::Type& record) {
    if (!record.isRecord() || record.isCompleteRecord()) {
        return;
    }
    std::vector<type::MemberSpec> specs;
    specs.reserve(record.getMembers().size());
    for (const auto& member : record.getMembers()) {
        type::Type memberType = member.type ? *member.type : type::voidType();
        visitVariableBounds(memberType, *this);
        if (memberType.isRecord()) {
            finalizeRecordDefinition(memberType);
        }
        memberType = ast::foldConstantArrayBounds(memberType);
        if (type::hasRuntimeSize(memberType)) {
            semanticError("array size is not a non-negative constant expression",
                    arrayBoundContext(memberType));
            return;
        }
        std::optional<int> bitWidth;
        if (member.isBitField()) {
            bitWidth = member.bitField->width;
        }
        specs.emplace_back(member.name, std::move(memberType), bitWidth);
    }
    type::recompleteRecord(record, std::move(specs));
}

void SemanticAnalysisVisitor::finalizeSpecifierType(ast::TypeSpecifier& spec) {
    spec.resolveTypeof(*this);
    if (!spec.hasType()) {
        return;
    }
    visitVariableBounds(spec.getType(), *this);
    spec.refoldConstantArrayBounds();
    if (spec.definesRecord()) {
        type::Type record = spec.getType();
        finalizeRecordDefinition(record);
    }
}

std::optional<type::Type> SemanticAnalysisVisitor::resolveTypeName(ast::TypeName& typeName,
        const translation_unit::Context& errorContext) {
    finalizeSpecifierType(typeName.spec);
    if (!typeName.spec.hasType()) {
        semanticError("cannot determine type of typeof operand", errorContext);
        return std::nullopt;
    }
    type::Type result = typeName.applyDad(typeName.spec.getType(), *this);
    visitVariableBounds(result, *this);
    return result;
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !containsSemanticErrors;
}

std::map<std::string, std::string> SemanticAnalysisVisitor::getConstants() const {
    return symbolTable.getConstants();
}

std::vector<symbols::ValueEntry> SemanticAnalysisVisitor::getDataHomes() const {
    return symbolTable.getDataHomes();
}


// Inserts ordinary FunctionEntry symbols for designator / & uses.
// Call-form builtins go through builtins::lookupBuiltin -> BuiltinPlan.
void SemanticAnalysisVisitor::installGnuBuiltins() {
    if (!gnuExtensions_) {
        return;
    }
    const translation_unit::Context ctx { "<gnu>", 0 };
    builtins::forEachDesignatorBuiltin([&](const char* name, const builtins::BuiltinDescriptor& desc) {
        type::Type fn = type::function(desc.returnType, { desc.argType });
        symbolTable.insertFunction(name, fn.getFunction(), ctx, false);
    });
}

} // namespace semantic_analyzer
