#include "SemanticAnalysisVisitorInternal.h"

#include "ast/InitializerListExpression.h"
#include "ast/IdentifierExpression.h"

namespace semantic_analyzer {

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    declarationSpecifiers.resolveTypeof(*this);
    if (declarationSpecifiers.getStorageSpecifiers().size() > 1) {
        semanticError("multiple storage classes in declaration specifiers",
                declarationSpecifiers.getStorageSpecifiers().at(1).getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    declaration.visitSpecifiers(*this);

    const auto& declSpecs = declaration.getDeclarationSpecifiers();
    if (declSpecs.isTypedef()) {
        // Type alias only: visit declarators; skip runtime symbol and initializers
        // (invalid `typedef int x = 1;` is not diagnosed on this path).
        for (const auto& declarator : declaration.getDeclarators()) {
            declarator->visitDeclarator(*this);
            checkObjectArrayBounds(*declarator);
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
    const IncompleteArrayBound bound =
            incompleteArrayBoundFromInitializer(declarator.getInitializer());
    if (bound.kind == IncompleteArrayBound::Kind::Error) {
        semanticError(bound.error, declarator.getContext());
        return false;
    }
    if (bound.kind == IncompleteArrayBound::Kind::None) {
        return true;
    }
    try {
        type = type::array(type.getElementType(), bound.bound);
    } catch (const std::invalid_argument& ex) {
        semanticError(ex.what(), declarator.getContext());
        return false;
    }
    return true;
}

void SemanticAnalysisVisitor::analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
        const ast::DeclarationSpecifiers& specifiers) {
    declarator.visitDeclarator(*this);
    checkObjectArrayBounds(declarator);

    const type::Type baseType = specifiers.getResolvedType();
    symbols::Storage storage = symbols::Storage::Automatic;
    if (symbolTable.isAtFileScope()) {
        if (specifiers.hasStorage(ast::Storage::STATIC)) {
            storage = symbols::Storage::Static;
        } else if (specifiers.hasStorage(ast::Storage::EXTERN) && !declarator.hasInitializer()) {
            storage = symbols::Storage::Extern;
        } else {
            storage = symbols::Storage::Global;
        }
    } else if (specifiers.hasStorage(ast::Storage::STATIC)) {
        storage = symbols::Storage::Static;
    }

    type::Type type { type::voidType() };
    bool typeOk = true;
    try {
        type = declarator.getFundamentalType(baseType);
    } catch (const std::invalid_argument& ex) {
        // array size overflow, array of incomplete type, etc.
        semanticError(ex.what(), declarator.getContext());
        typeOk = false;
    }
    bool initializerVisited = false;
    if (typeOk && !rewriteCharArrayStringInitializer(declarator, type)) {
        typeOk = false;
    }
    if (typeOk && !completeArrayFromInitializer(declarator, type, initializerVisited)) {
        typeOk = false;
    }

    bool inserted = false;
    if (typeOk) {
        if (type.isVoid()) {
            semanticError("variable `" + declarator.getName() + "` declared void", declarator.getContext());
        } else if (type.isIncompleteArray() && storage != symbols::Storage::Extern) {
            semanticError("variable `" + declarator.getName() + "` has incomplete type",
                    declarator.getContext());
        } else if (type.isIncompleteRecord()) {
            semanticError("variable `" + declarator.getName() + "` has incomplete type", declarator.getContext());
        } else if (type.isFunction()) {
            // Prototypes: register with resolved return type (FunctionDeclarator no longer inserts).
            if (symbolTable.hasEnumConstant(declarator.getName()) && symbolTable.isAtFileScope()) {
                semanticError("redefinition of enumerator `" + declarator.getName() + "` as a function",
                        declarator.getContext());
            } else if (symbolTable.hasGlobalVariable(declarator.getName())) {
                semanticError("function `" + declarator.getName()
                                + "` conflicts with global variable of the same name",
                        declarator.getContext());
            } else if (symbolTable.hasFunction(declarator.getName())) {
                auto existing = symbolTable.findFunction(declarator.getName());
                if (!functionTypesCompatible(existing.getType(), type.getFunction())) {
                    semanticError("function `" + declarator.getName()
                                    + "` declaration conflicts with previous one on "
                                    + to_string(existing.getContext()),
                            declarator.getContext());
                } else if (staticFollowsNonStatic(existing.hasInternalLinkage(),
                        specifiers.hasStorage(ast::Storage::STATIC))) {
                    semanticError(staticFollowsNonStaticMessage(declarator.getName()),
                            declarator.getContext());
                }
            } else {
                symbolTable.insertFunction(declarator.getName(), type.getFunction(),
                        declarator.getContext(), specifiers.hasStorage(ast::Storage::STATIC));
            }
        } else if (symbolTable.isAtFileScope() && symbolTable.hasFunction(declarator.getName())) {
            semanticError("symbol `" + declarator.getName() + "` declaration conflicts with function of the same name",
                    declarator.getContext());
        } else if (symbolTable.hasEnumConstant(declarator.getName()) && symbolTable.isAtFileScope()) {
            // File-scope ordinary identifiers share a namespace with enumerators (C).
            semanticError("redefinition of enumerator `" + declarator.getName() + "`",
                    declarator.getContext());
        } else if (symbolTable.insertSymbol(declarator.getName(), type, declarator.getContext(),
                storage)) {
            declarator.setHolder(annotations(), symbolTable.lookup(declarator.getName()));
            inserted = true;
        } else if (symbolTable.isAtFileScope()
                && staticFollowsNonStatic(symbolTable.lookup(declarator.getName()).isStatic(),
                        storage == symbols::Storage::Static)) {
            semanticError(staticFollowsNonStaticMessage(declarator.getName()),
                    declarator.getContext());
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
        if (inserted) {
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

void SemanticAnalysisVisitor::checkObjectArrayBounds(ast::InitializedDeclarator& declarator) {
    bool tooLarge = false;
    bool unfixed = false;
    declarator.forEachArrayDeclarator([&](ast::ArrayDeclarator& array) {
        if (!array.subscriptExpression || array.hasArraySize()) {
            return;
        }
        array.subscriptExpression->accept(*this);
        const ast::ArrayBoundFold folded = array.foldOwnBound();
        if (folded == ast::ArrayBoundFold::TooLarge) {
            tooLarge = true;
        } else if (folded == ast::ArrayBoundFold::Unfixed) {
            unfixed = true;
        }
    });
    if (tooLarge) {
        semanticError("array size is too large", declarator.getContext());
        return;
    }
    if (unfixed) {
        semanticError("array size is not a non-negative constant expression",
                declarator.getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);

    argumentNames.clear();
    for (auto& argumentDeclaration : declarator.getFormalArguments()) {
        argumentNames.push_back(argumentDeclaration.getName());
    }
    // Registration happens in visit(Declaration) for prototypes or visit(FunctionDefinition)
    // for definitions, once the full return type is known via getResolvedType.
}

void SemanticAnalysisVisitor::visit(ast::FormalArgument& argument) {
    argument.visitSpecifiers(*this);
    argument.visitDeclarator(*this);
    type::Type type { type::voidType() };
    try {
        type = argument.getType();
    } catch (const std::invalid_argument& ex) {
        semanticError(ex.what(), argument.getDeclarationContext());
        return;
    }
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
    if (symbolTable.hasGlobalVariable(function.getName())) {
        semanticError("function `" + function.getName() + "` conflicts with global variable of the same name",
                function.getDeclaratorContext());
        return;
    }
    if (symbolTable.hasFunction(function.getName())) {
        FunctionEntry existing = symbolTable.findFunction(function.getName());
        if (symbolTable.isFunctionDefined(function.getName())) {
            semanticError("function `" + function.getName()
                            + "` definition conflicts with previous one on "
                            + to_string(existing.getContext()),
                    function.getDeclaratorContext());
            return;
        }
        if (!functionTypesCompatible(existing.getType(), functionType.getFunction())) {
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
        symbolTable.updateFunction(function.getName(), functionType.getFunction(),
                function.getDeclaratorContext());
    } else {
        symbolTable.insertFunction(function.getName(), functionType.getFunction(),
                function.getDeclaratorContext(),
                function.getReturnTypeSpecifiers().hasStorage(ast::Storage::STATIC));
    }
    symbolTable.markFunctionDefined(function.getName());
    function.setSymbol(symbolTable.findFunction(function.getName()));
    currentReturnType = functionType.getFunction().getReturnType();
    currentFunctionName = function.getName();
    symbolTable.startFunction(function.getName(), argumentNames);
    namedLabels.clear();
    pendingGotos.clear();
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
    function.setArguments(symbolTable.getCurrentScopeArguments());
    function.setLocalVariables(symbolTable.getCurrentScopeSymbols());
    symbolTable.endFunction();
    currentReturnType.reset();
    currentFunctionName.clear();
}

void SemanticAnalysisVisitor::visit(ast::Block& block) {
    symbolTable.enterBlockScope();
    block.visitChildren(*this);
    symbolTable.exitBlockScope();
}

void SemanticAnalysisVisitor::typeCheck(const type::Type& typeFrom, const type::Type& typeTo,
        const translation_unit::Context& context)
{
    if (typeTo.canAssignFrom(typeFrom)) {
        return;
    }
    semanticError(type::productAssignFailureMessage(typeTo, typeFrom), context);
}

void SemanticAnalysisVisitor::rejectFunctionValue(const type::Type& type, const translation_unit::Context& context) {
    if (type::isBareFunction(type)) {
        semanticError("function designator used as a value is not supported", context);
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

void SemanticAnalysisVisitor::importParseEnumConstant(const std::string& name, long value) {
    // defineEnumConstant no-ops on redefinition; sole SA import channel.
    symbolTable.defineEnumConstant(name, value);
}

} // namespace semantic_analyzer
