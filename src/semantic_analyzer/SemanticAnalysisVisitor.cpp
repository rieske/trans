#include "SemanticAnalysisVisitorInternal.h"

#include "ast/DeclarationSpecifiers.h"
#include "ast/FormalArgument.h"
#include "ast/FunctionDeclarator.h"
#include "ast/FunctionDefinition.h"
#include "ast/GnuBuiltinFunctions.h"
#include "ast/TypeSpecifier.h"
#include "ast/VlaExpressionTable.h"
#include "translation_unit/Context.h"
#include "util/Diagnostic.h"

#include <stdexcept>

namespace semantic_analyzer {

const ast::VlaExpressionTable& SemanticAnalysisVisitor::vlaTable() const {
    if (!vlas_) {
        throw std::logic_error { "missing VLA expression table" };
    }
    return *vlas_;
}

diag::Sink& SemanticAnalysisVisitor::sink() const {
    if (!sink_) {
        throw std::logic_error { "missing diagnostic sink" };
    }
    return *sink_;
}

namespace {

translation_unit::Context arrayBoundContext(const type::Type& t, const ast::VlaExpressionTable& vlas) {
    type::Type walk = t;
    while (walk.isArray()) {
        if (auto bound = vlas.lookup(walk.vlaBound().get())) {
            return bound->getContext();
        }
        walk = walk.getElementType();
    }
    if (walk.isPointer()) {
        return arrayBoundContext(walk.dereference(), vlas);
    }
    return translation_unit::Context { "", 0 };
}

} // namespace

void finalizeRecordDefinition(type::Type& record, SemanticAnalysisVisitor& visitor,
        const translation_unit::Context& where) {
    if (!record.isRecord() || record.isCompleteRecord()) {
        return;
    }
    auto specs = type::memberSpecs(record);
    for (auto& spec : specs) {
        const auto& vlas = visitor.vlaTable();
        visitVariableBounds(spec.type, visitor, vlas);
        if (spec.type.isRecord()) {
            finalizeRecordDefinition(spec.type, visitor, where);
        }
        spec.type = ast::foldConstantArrayBounds(spec.type, vlas);
        if (type::hasRuntimeSize(spec.type)) {
            visitor.semanticError("array size is not a non-negative constant expression",
                    arrayBoundContext(spec.type, vlas));
        }
    }
    if (const char* error = type::relayoutFromMemberSpecs(record, specs)) {
        translation_unit::Context at = where;
        if (at.getOffset() == 0 && at.getSourceName().empty()) {
            at = arrayBoundContext(record, visitor.vlaTable());
        }
        visitor.semanticError(error, at);
    }
}

void resolveSpecifierType(ast::TypeSpecifier& spec, SemanticAnalysisVisitor& visitor) {
    spec.resolveTypeof(visitor);
    if (!spec.hasType()) {
        return;
    }
    const auto& vlas = visitor.vlaTable();
    visitVariableBounds(spec.getType(), visitor, vlas);
    spec.refoldConstantArrayBounds(vlas);
    if (spec.definesRecord()) {
        type::Type record = spec.getType();
        finalizeRecordDefinition(record, visitor, spec.getContext());
    }
}

void finalizeSpecifierType(ast::TypeSpecifier& spec, SemanticAnalysisVisitor& visitor) {
    visitor.declareEnumerators(spec);
    resolveSpecifierType(spec, visitor);
}

void applyIncomingFunctionSpecs(SymbolTable& table, const std::string& name,
        const ast::DeclarationSpecifiers& specs) {
    table.applyFunctionSpecifiers(name,
            specs.hasFunctionSpec(ast::FunctionSpec::INLINE),
            specs.hasFunctionSpec(ast::FunctionSpec::NORETURN),
            specs.hasStorage(ast::Storage::EXTERN));
}

void analyzeSpecifiers(ast::DeclarationSpecifiers& specifiers, SemanticAnalysisVisitor& visitor) {
    if (specifiers.getStorageSpecifiers().size() > 1) {
        visitor.semanticError("multiple storage classes in declaration specifiers",
                specifiers.getStorageSpecifiers().at(1).getContext());
    }
    for (auto& specifier : specifiers.getTypeSpecifiers()) {
        resolveSpecifierType(specifier, visitor);
    }
}

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    for (const auto& specifier : declarationSpecifiers.getTypeSpecifiers()) {
        declareEnumerators(specifier);
    }
    analyzeSpecifiers(declarationSpecifiers, *this);
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    declaration.visitSpecifiers(*this);

    const auto& declSpecs = declaration.getDeclarationSpecifiers();
    if (declSpecs.isTypedef()) {
        if (!declSpecs.getFunctionSpecifiers().empty() && !declaration.getDeclarators().empty()) {
            semanticError("function specifier may only appear in a function declaration",
                    declaration.getDeclarators().front()->getContext());
        }
        // Type alias only: no runtime symbol, so there is nothing for an initializer to
        // initialize.
        for (const auto& declarator : declaration.getDeclarators()) {
            declarator->visitDeclarator(*this);
            if (declarator->hasInitializer()) {
                semanticError("typedef `" + declarator->getName() + "` is initialized",
                        declarator->getContext());
            }
            checkObjectArrayBounds(*declarator, !symbolTable.isAtFileScope());
            type::Type aliased = declarator->getFundamentalType(declSpecs.getResolvedType());
            if (const char* error = declarator->getDeclarator().arrayConstraintError(aliased)) {
                semanticError(error, declarator->getContext());
            }
        }
        return;
    }

    // C: each declarator is visible to later initializers in the same declaration
    // (`int a = 1, b = a;`). Insert before walking the initializer.
    for (const auto& declarator : declaration.getDeclarators()) {
        analyzeInitializedDeclarator(*declarator, declSpecs);
    }
}

void SemanticAnalysisVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void SemanticAnalysisVisitor::visit(ast::InitializedDeclarator&) {
    // Bare accept cannot supply the Declaration's base type; use analyzeInitializedDeclarator.
    throw std::logic_error(
            "InitializedDeclarator: use analyzeInitializedDeclarator(specifiers), not bare accept");
}

bool SemanticAnalysisVisitor::completeArrayFromInitializer(ast::InitializedDeclarator& declarator,
        type::Type& type, bool& initializerVisited) {
    if (!type.isIncompleteArray() || !declarator.hasInitializer()) {
        return true;
    }
    declarator.visitInitializer(*this);
    initializerVisited = true;
    return applyIncompleteArrayBound(type, declarator.getInitializer(), declarator.getContext());
}

void SemanticAnalysisVisitor::analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
        const ast::DeclarationSpecifiers& specifiers) {
    declarator.visitDeclarator(*this);

    const type::Type baseType = specifiers.getResolvedType();
    // C: static / extern apply at file or block scope; bare file-scope is a definition
    // (or tentative definition). Pure extern (no initializer) never allocates here.
    bool typeOk = true;
    symbols::Storage storage = symbols::Storage::Automatic;
    if (specifiers.hasStorage(ast::Storage::STATIC)) {
        storage = symbols::Storage::Static;
    } else if (specifiers.hasStorage(ast::Storage::EXTERN)) {
        if (!declarator.hasInitializer()) {
            storage = symbols::Storage::Extern;
        } else if (symbolTable.isAtFileScope()) {
            storage = symbols::Storage::Global;
        } else {
            semanticError("`" + declarator.getName() + "` has both `extern` and initializer",
                    declarator.getContext());
            typeOk = false;
        }
    } else if (symbolTable.isAtFileScope()) {
        storage = symbols::Storage::Global;
    }
    checkObjectArrayBounds(declarator, storage == symbols::Storage::Automatic);

    type::Type type = declarator.getFundamentalType(baseType);
    if (const char* error = declarator.getDeclarator().arrayConstraintError(type)) {
        semanticError(error, declarator.getContext());
        typeOk = false;
    }
    if (typeOk && type.isFunction() && declarator.hasInitializer()) {
        semanticError("function `" + declarator.getName() + "` is initialized like a variable",
                declarator.getContext());
        typeOk = false;
    }
    if (!specifiers.getFunctionSpecifiers().empty() && !type.isFunction()) {
        semanticError("function specifier may only appear in a function declaration",
                declarator.getContext());
        typeOk = false;
    }
    bool initializerVisited = false;
    if (typeOk && !rewriteCharArrayStringInitializer(declarator, type)) {
        typeOk = false;
    }
    if (typeOk && !completeArrayFromInitializer(declarator, type, initializerVisited)) {
        typeOk = false;
    }

    bool bound = false;
    if (typeOk) {
        if (type.isVoid()) {
            semanticError("variable `" + declarator.getName() + "` declared void", declarator.getContext());
        } else if ((type.isIncompleteArray() || type.isIncompleteRecord())
                && storage != symbols::Storage::Extern) {
            // pure extern may be incomplete
            semanticError("variable `" + declarator.getName() + "` has incomplete type",
                    declarator.getContext());
        } else if (type.isFunction()) {
            // Prototypes: register with resolved return type (FunctionDeclarator no longer inserts).
            const auto* fileScope = symbolTable.findFileScope(declarator.getName());
            if (symbolTable.isAtFileScope() && fileScope && fileScope->isEnumerator()) {
                semanticError("redefinition of enumerator `" + declarator.getName() + "` as a function",
                        declarator.getContext());
            } else if (symbolTable.hasGlobalVariable(declarator.getName())) {
                semanticError("function `" + declarator.getName()
                                + "` conflicts with global variable of the same name",
                        declarator.getContext());
            } else if (symbolTable.hasFunction(declarator.getName())) {
                auto existing = symbolTable.findFunction(declarator.getName());
                if (!type.compatibleWith(existing.getType())) {
                    semanticError("function `" + declarator.getName()
                                    + "` declaration conflicts with previous one on "
                                    + to_string(existing.getContext()),
                            declarator.getContext());
                } else if (staticFollowsNonStatic(existing.hasInternalLinkage(),
                        specifiers.hasStorage(ast::Storage::STATIC))) {
                    semanticError(staticFollowsNonStaticMessage(declarator.getName()),
                            declarator.getContext());
                } else {
                    applyIncomingFunctionSpecs(symbolTable, declarator.getName(), specifiers);
                }
            } else {
                symbolTable.insertFunction(declarator.getName(), type,
                        declarator.getContext(), specifiers.hasStorage(ast::Storage::STATIC));
                applyIncomingFunctionSpecs(symbolTable, declarator.getName(), specifiers);
            }
        } else if (symbolTable.isAtFileScope() && symbolTable.hasFunction(declarator.getName())) {
            semanticError("symbol `" + declarator.getName() + "` declaration conflicts with function of the same name",
                    declarator.getContext());
        } else if (symbolTable.isAtFileScope()) {
            const ObjectBind result = symbolTable.bindFileScopeObject(declarator.getName(), type,
                    declarator.getContext(), storage, declarator.hasInitializer());
            switch (result) {
            case ObjectBind::Bound:
                declarator.setHolder(annotations(), symbolTable.lookup(declarator.getName()));
                bound = true;
                break;
            case ObjectBind::StaticAfterNonStatic:
                semanticError(staticFollowsNonStaticMessage(declarator.getName()),
                        declarator.getContext());
                break;
            case ObjectBind::NonStaticAfterStatic:
                semanticError(nonStaticFollowsStaticMessage(declarator.getName()),
                        declarator.getContext());
                break;
            case ObjectBind::TypeConflict:
            case ObjectBind::SecondDefinition:
                if (const auto* existing = symbolTable.findFileScope(declarator.getName());
                        existing && existing->isEnumerator()) {
                    semanticError("redefinition of enumerator `" + declarator.getName() + "`",
                            declarator.getContext());
                } else {
                    semanticError(
                            "symbol `" + declarator.getName() +
                                    "` declaration conflicts with previous declaration on " +
                                    to_string(symbolTable.lookup(declarator.getName()).getContext()),
                            declarator.getContext());
                }
                break;
            }
        } else if (symbolTable.insertSymbol(declarator.getName(), type, declarator.getContext(),
                storage)) {
            declarator.setHolder(annotations(), symbolTable.lookup(declarator.getName()));
            bound = true;
        } else {
            semanticError(
                    "symbol `" + declarator.getName() +
                            "` declaration conflicts with previous declaration on " +
                            to_string(symbolTable.lookup(declarator.getName()).getContext()),
                    declarator.getContext());
        }
    }

    if (declarator.hasInitializer()) {
        if (!initializerVisited) {
            declarator.visitInitializer(*this);
        }
        if (bound) {
            lowerLocalInitializer(declarator, type);
        }
    }
}

void SemanticAnalysisVisitor::visit(ast::Pointer&) {
}

void SemanticAnalysisVisitor::visit(ast::Identifier&) {
}

void SemanticAnalysisVisitor::visit(ast::ArrayDeclarator& declaration) {
    declaration.visitBaseDeclarator(*this);
    if (declaration.foldOwnBound() == ast::ArrayBoundFold::TooLarge) {
        semanticError("array size is too large", declaration.getContext());
    }
}

void SemanticAnalysisVisitor::checkObjectArrayBounds(ast::InitializedDeclarator& declarator,
        bool allowVla) {
    bool tooLarge = false;
    bool negative = false;
    bool unfixed = false;
    declarator.forEachArrayDeclarator([&](ast::ArrayDeclarator& array) {
        if (!array.subscriptExpression || array.hasArraySize()) {
            return;
        }
        array.subscriptExpression->accept(*this);
        const ast::ArrayBoundFold folded = array.foldOwnBound();
        if (folded == ast::ArrayBoundFold::TooLarge) {
            tooLarge = true;
        } else if (folded == ast::ArrayBoundFold::Negative) {
            negative = true;
        } else if (folded == ast::ArrayBoundFold::Unfixed) {
            unfixed = true;
        }
    });
    if (tooLarge) {
        semanticError("array size is too large", declarator.getContext());
        return;
    }
    if (negative || (unfixed && !allowVla)) {
        semanticError("array size is not a non-negative constant expression",
                declarator.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
    declarator.visitNestedDeclarator(*this);
}

void SemanticAnalysisVisitor::visit(ast::FormalArgument& argument) {
    analyzeSpecifiers(argument.getSpecifiers(), *this);
    if (!argument.getSpecifiers().getFunctionSpecifiers().empty()) {
        semanticError("function specifier may not appear in a parameter declaration",
                argument.getDeclarationContext());
    }
    argument.visitDeclarator(*this);
    type::Type type = argument.getType();
    if (type.isVoid()) {
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
    if (const char* error = function.getDeclarator().arrayConstraintError(functionType)) {
        semanticError(error, function.getDeclaratorContext());
        return;
    }
    if (symbolTable.hasGlobalVariable(function.getName())) {
        semanticError("function `" + function.getName() + "` conflicts with global variable of the same name",
                function.getDeclaratorContext());
        return;
    }
    if (symbolTable.hasFunction(function.getName())) {
        symbols::FunctionEntry existing = symbolTable.findFunction(function.getName());
        if (symbolTable.isFunctionDefined(function.getName())) {
            semanticError("function `" + function.getName()
                            + "` definition conflicts with previous one on "
                            + to_string(existing.getContext()),
                    function.getDeclaratorContext());
            return;
        }
        if (!functionType.compatibleWith(existing.getType())) {
            semanticError("function `" + function.getName()
                            + "` definition conflicts with previous one on "
                            + to_string(existing.getContext()),
                    function.getDeclaratorContext());
            return;
        }
        if (staticFollowsNonStatic(existing.hasInternalLinkage(),
                function.getReturnTypeSpecifiers().hasStorage(ast::Storage::STATIC))) {
            semanticError(staticFollowsNonStaticMessage(function.getName()),
                    function.getDeclaratorContext());
            return;
        }
        symbolTable.updateFunction(function.getName(), functionType,
                function.getDeclaratorContext());
        applyIncomingFunctionSpecs(symbolTable, function.getName(),
                function.getReturnTypeSpecifiers());
    } else {
        symbolTable.insertFunction(function.getName(), functionType,
                function.getDeclaratorContext(),
                function.getReturnTypeSpecifiers().hasStorage(ast::Storage::STATIC));
        applyIncomingFunctionSpecs(symbolTable, function.getName(),
                function.getReturnTypeSpecifiers());
    }
    symbolTable.markFunctionDefined(function.getName());
    symbolTable.startFunction(function.getName(), function.definedFunctionParameterNames());
    namedLabels.clear();
    pendingGotos.clear();
    if (const auto* fn = function.definedFunctionDeclarator()) {
        for (const auto& argument : fn->getFormalArguments()) {
            for (const auto& specifier : argument.getSpecifiers().getTypeSpecifiers()) {
                declareEnumerators(specifier);
            }
        }
    }
    // Parameters and outermost body declarations share one scope (C); do not enterBlockScope.
    function.visitBodyChildren(*this);
    for (auto* gotoStmt : pendingGotos) {
        auto it = namedLabels.find(gotoStmt->getLabelName());
        if (it == namedLabels.end()) {
            semanticError("label `" + gotoStmt->getLabelName() + "` used but not defined",
                    gotoStmt->label.context);
        } else {
            gotoStmt->setTarget(annotations(), it->second);
        }
    }
    namedLabels.clear();
    pendingGotos.clear();
    annotations().setFunctionFrame(&function, symbols::FunctionFrame {
            symbolTable.currentFunctionEntry(),
            symbolTable.getCurrentScopeSymbols(),
            symbolTable.getCurrentScopeArguments() });
    symbolTable.endFunction();
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    symbolTable.enterBlockScope();
    block.visitChildren(*this);
    symbolTable.exitBlockScope();
}

bool SemanticAnalysisVisitor::checkAssign(const type::Type& dest, const type::Type& source,
        const translation_unit::Context& context, const ast::Expression* sourceExpr)
{
    if (productAssignOk(dest, source, sourceExpr)) {
        return true;
    }
    semanticError(type::productAssignFailureMessage(dest, source), context);
    return false;
}

bool SemanticAnalysisVisitor::checkOperandTypes(const type::Type& left, const type::Type& right,
        const translation_unit::Context& context)
{
    // Historical product gate: accept when right can accept left (type-only).
    return checkAssign(right, left, context, nullptr);
}

void SemanticAnalysisVisitor::declareEnumerators(const ast::TypeSpecifier& specifier) {
    for (const auto& enumerator : specifier.enumerators()) {
        symbolTable.insertEnumerator(enumerator.name, enumerator.value);
    }
}

void SemanticAnalysisVisitor::rejectFunctionValue(const type::Type& type, const translation_unit::Context& context) {
    if (type.isFunction()) {
        semanticError("function designator used as a value is not supported", context);
    }
}

// C: the contexts that require a scalar value.
void SemanticAnalysisVisitor::checkScalarValue(ast::Expression& expression) {
    if (!expression.hasExpressionType()) {
        return;
    }
    if (!type::isProductScalar(type::afterLvalueConversion(expression.expressionType()))) {
        semanticError("used a non-scalar value where a scalar is required", expression.getContext());
    }
}

// Same rule where the value is also taken: arrays decay to pointers first.
void SemanticAnalysisVisitor::requireScalarValue(ast::Expression& expression) {
    decayArrayValue(expression, symbolTable, annotations());
    checkScalarValue(expression);
}

void SemanticAnalysisVisitor::semanticError(std::string message, const translation_unit::Context& context) {
    sink().error(context, std::move(message));
}

bool SemanticAnalysisVisitor::successfulSemanticAnalysis() const {
    return !sink().hasErrors();
}

std::map<std::string, std::string> SemanticAnalysisVisitor::getConstants() const {
    return symbolTable.getConstants();
}

std::vector<symbols::ValueEntry> SemanticAnalysisVisitor::getDataHomes() const {
    return symbolTable.getDataHomes();
}

void SemanticAnalysisVisitor::installGnuBuiltins() {
    if (!gnuExtensions_) {
        return;
    }
    const translation_unit::Context ctx { "<gnu>", 0 };
    for (const auto& builtin : ast::kGnuBswapBuiltins) {
        type::Type value = ast::gnuBswapValueType(builtin.widthBytes);
        type::Type fn = type::function(value, { value });
        symbolTable.insertFunction(builtin.name, fn, ctx, false);
    }
    for (const auto& builtin : ast::kGnuCtzBuiltins) {
        type::Type fn = type::function(type::signedInteger(), { ast::gnuCtzArgType(builtin.widthBytes) });
        symbolTable.insertFunction(builtin.name, fn, ctx, false);
    }
    type::Type allocaFn = type::function(type::pointer(type::voidType()), { type::unsignedLong() });
    symbolTable.insertFunction("__builtin_alloca", allocaFn, ctx, false);
}

} // namespace semantic_analyzer
