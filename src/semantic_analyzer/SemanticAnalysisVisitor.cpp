#include "SemanticAnalysisVisitorInternal.h"

#include "ast/InitializerListExpression.h"
#include "ast/IdentifierExpression.h"

namespace semantic_analyzer {

SemanticAnalysisVisitor::SemanticAnalysisVisitor() {
    type::Type functionType = type::function(type::signedInteger());
    symbolTable.insertFunction("printf", functionType.getFunction(), externalContext());
    symbolTable.insertFunction("scanf", functionType.getFunction(), externalContext());
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
}

void SemanticAnalysisVisitor::printSymbolTable() const {
    symbolTable.printTable();
}

void SemanticAnalysisVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    // FIXME: this would look so much better
    /*for (std::string error : declarationSpecifiers.getSemanticErrors()) {
     semanticError(error, globalContext);
     }*/
    if (declarationSpecifiers.getStorageSpecifiers().size() > 1) {
        semanticError("multiple storage classes in declaration specifiers",
                declarationSpecifiers.getStorageSpecifiers().at(1).getContext());
    }
}

void SemanticAnalysisVisitor::visit(ast::Declaration& declaration) {
    declaration.visitSpecifiers(*this);

    const type::Type baseType =
            declaration.getDeclarationSpecifiers().getTypeSpecifiers().at(0).getType();
    // C: each declarator is visible to later initializers in the same declaration
    // (`int a = 1, b = a;`). Insert before walking the initializer.
    for (const auto& declarator : declaration.getDeclarators()) {
        analyzeInitializedDeclarator(*declarator, baseType);
    }
}

void SemanticAnalysisVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void SemanticAnalysisVisitor::visit(ast::InitializedDeclarator&) {
    // Bare accept cannot supply the Declaration's base type; use analyzeInitializedDeclarator.
    throw std::logic_error(
            "InitializedDeclarator: use analyzeInitializedDeclarator(baseType), not bare accept");
}

void SemanticAnalysisVisitor::analyzeInitializedDeclarator(ast::InitializedDeclarator& declarator,
        const type::Type& baseType) {
    declarator.visitDeclarator(*this);

    type::Type type { type::voidType() };
    bool typeOk = true;
    try {
        type = declarator.getFundamentalType(baseType);
    } catch (const std::invalid_argument& ex) {
        // array size overflow, array of incomplete type, etc.
        semanticError(ex.what(), declarator.getContext());
        typeOk = false;
    }

    bool inserted = false;
    if (typeOk) {
        if (type.isVoid()) {
            semanticError("variable `" + declarator.getName() + "` declared void", declarator.getContext());
        } else if (type.isIncompleteStructure()) {
            semanticError("variable `" + declarator.getName() + "` has incomplete type", declarator.getContext());
        } else if (symbolTable.isAtFileScope() && symbolTable.hasFunction(declarator.getName())) {
            semanticError("symbol `" + declarator.getName() + "` declaration conflicts with function of the same name",
                    declarator.getContext());
        } else if (symbolTable.insertSymbol(declarator.getName(), type, declarator.getContext())) {
            declarator.setHolder(symbolTable.lookup(declarator.getName()));
            inserted = true;
        } else {
            semanticError(
                    "symbol `" + declarator.getName() +
                            "` declaration conflicts with previous declaration on " +
                            to_string(symbolTable.lookup(declarator.getName()).getContext()),
                    declarator.getContext());
        }
    }

    // Always walk initializers (including error recovery); lower only when the name was inserted.
    if (declarator.hasInitializer()) {
        declarator.visitInitializer(*this);
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
    if (declaration.subscriptExpression) {
        declaration.subscriptExpression->accept(*this);
        long length = 0;
        if (!declaration.subscriptExpression->evaluateConstant(length) || length < 0) {
            semanticError("array size is not a non-negative constant expression",
                    declaration.getContext());
            declaration.setArraySize(0);
        } else if (length > static_cast<long>(std::numeric_limits<int>::max())) {
            semanticError("array size is too large", declaration.getContext());
            declaration.setArraySize(0);
        } else {
            declaration.setArraySize(length);
        }
    } else {
        // Incomplete array T a[] — treat as zero-length for now.
        declaration.setArraySize(0);
    }
}

void SemanticAnalysisVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);

    argumentNames.clear();
    std::vector<type::Type> arguments;
    for (auto& argumentDeclaration : declarator.getFormalArguments()) {
        try {
            arguments.push_back(argumentDeclaration.getType());
        } catch (const std::invalid_argument&) {
            // visit(FormalArgument) already diagnosed; placeholder so analysis can finish.
            arguments.push_back(type::voidType());
        }
        argumentNames.push_back(argumentDeclaration.getName());
    }

    // FIXME: return type is not known at this point!
    type::Type functionType = type::function(type::signedInteger(), arguments);
    if (symbolTable.hasGlobalVariable(declarator.getName())) {
        semanticError("function `" + declarator.getName() + "` conflicts with global variable of the same name",
                declarator.getContext());
        return;
    }
    FunctionEntry functionEntry = symbolTable.insertFunction(
            declarator.getName(),
            functionType.getFunction(),
            declarator.getContext());

    if (functionEntry.getContext() != declarator.getContext()) {
        semanticError("function `" + declarator.getName() + "` definition conflicts with previous one on "
                + to_string(functionEntry.getContext()), declarator.getContext());
    }
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
    function.visitDeclarator(*this);

    if (!symbolTable.hasFunction(function.getName())) {
        return;
    }
    function.setSymbol(symbolTable.findFunction(function.getName()));
    currentReturnType = function.getSymbol()->returnType();
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

std::vector<ValueEntry> SemanticAnalysisVisitor::getGlobalVariables() const {
    return symbolTable.getGlobalVariables();
}


} // namespace semantic_analyzer
