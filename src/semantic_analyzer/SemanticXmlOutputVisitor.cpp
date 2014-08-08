#include "SemanticXmlOutputVisitor.h"

#include <algorithm>
#include <iterator>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "ArrayDeclaration.h"
#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"
#include "BitwiseExpression.h"
#include "Block.h"
#include "ComparisonExpression.h"
#include "DeclarationList.h"
#include "ExpressionList.h"
#include "ForLoopHeader.h"
#include "FunctionCall.h"
#include "FunctionDeclaration.h"
#include "FunctionDefinition.h"
#include "Identifier.h"
#include "IfElseStatement.h"
#include "IfStatement.h"
#include "IOStatement.h"
#include "JumpStatement.h"
#include "ListCarrier.h"
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "LoopStatement.h"
#include "NoArgFunctionCall.h"
#include "ParameterDeclaration.h"
#include "ParameterList.h"
#include "Pointer.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ReturnStatement.h"
#include "ShiftExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "TranslationUnit.h"
#include "TypeCast.h"
#include "UnaryExpression.h"
#include "VariableDeclaration.h"
#include "VariableDefinition.h"
#include "WhileLoopHeader.h"

static const std::string IDENTATION { "  " };

namespace semantic_analyzer {

SemanticXmlOutputVisitor::SemanticXmlOutputVisitor(std::ostream* outputStream) :
        outputStream { outputStream } {
}

SemanticXmlOutputVisitor::~SemanticXmlOutputVisitor() {
}

void SemanticXmlOutputVisitor::openXmlNode(const std::string& nodeName) {
    ident();
    *outputStream << "<" << stripLabel(nodeName) << ">\n";
    ++identation;
}

void SemanticXmlOutputVisitor::closeXmlNode(const std::string& nodeName) {
    --identation;
    ident();
    *outputStream << "</" << stripLabel(nodeName) << ">\n";
}

void SemanticXmlOutputVisitor::createLeafNode(std::string nodeName, std::string value) const {
    *outputStream << "<" << nodeName << ">" << value << "</" << nodeName << ">\n";
}

void SemanticXmlOutputVisitor::createLeafNode(std::string nodeName, std::string typeAttribute, std::string value) const {
    *outputStream << "<" << nodeName << " type='" << typeAttribute << "'>" << value << "</" << nodeName << ">\n";
}

std::string SemanticXmlOutputVisitor::stripLabel(std::string label) const {
    label.erase(std::remove(label.begin(), label.end(), '<'), label.end());
    label.erase(std::remove(label.begin(), label.end(), '>'), label.end());
    return label;
}

void SemanticXmlOutputVisitor::ident() {
    for (int i = 0; i != identation; ++i) {
        *outputStream << IDENTATION;
    }
}

void SemanticXmlOutputVisitor::visit(TypeSpecifier& typeSpecifier) {
    ident();
    createLeafNode("typeSpecifier", typeSpecifier.getName());
}

void SemanticXmlOutputVisitor::visit(ParameterList& parameterList) {
    const std::string nodeId { "parameterList" };
    openXmlNode(nodeId);
    for (const auto& parameter : parameterList.getDeclaredParameters()) {
        parameter->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(AssignmentExpressionList& expressions) {
    const std::string nodeId { "assignmentExpressions" };
    openXmlNode(nodeId);
    for (const auto& expression : expressions.getExpressions()) {
        expression->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(DeclarationList& declarations) {
    const std::string nodeId { "declarations" };
    openXmlNode(nodeId);
    for (const auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ArrayAccess& arrayAccess) {
    const std::string nodeId { "arrayAccess" };
    openXmlNode(nodeId);
    arrayAccess.postfixExpression->accept(*this);
    arrayAccess.subscriptExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(FunctionCall& functionCall) {
    const std::string nodeId { "functionCall" };
    openXmlNode(nodeId);
    functionCall.postfixExpression->accept(*this);
    functionCall.assignmentExpressionList->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(NoArgFunctionCall& functionCall) {
    const std::string nodeId { "noargFunctionCall" };
    openXmlNode(nodeId);
    functionCall.callExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(Term& term) {
    ident();
    createLeafNode("term", term.term.type, term.term.value);
}

void SemanticXmlOutputVisitor::visit(PostfixExpression& expression) {
    const std::string nodeId { "postfixExpression" };
    openXmlNode(nodeId);
    expression.postfixExpression->accept(*this);
    createLeafNode("operator", expression.postfixOperator.type);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(PrefixExpression& expression) {
    const std::string nodeId { "prefixExpression" };
    openXmlNode(nodeId);
    createLeafNode("operator", expression.incrementOperator.type);
    expression.unaryExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(UnaryExpression& expression) {
    const std::string nodeId { "unaryExpression" };
    openXmlNode(nodeId);
    createLeafNode("operator", expression.unaryOperator.type);
    expression.castExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(TypeCast& expression) {
    const std::string nodeId { "typeCast" };
    openXmlNode(nodeId);
    expression.typeSpecifier.accept(*this);
    expression.castExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(PointerCast& expression) {
    const std::string nodeId { "pointerCast" };
    openXmlNode(nodeId);
    expression.type.accept(*this);
    expression.pointer->accept(*this);
    expression.castExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ArithmeticExpression& expression) {
    const std::string nodeId { "arithmeticExpression" };
    openXmlNode(nodeId);
    expression.leftHandSide->accept(*this);
    ident();
    createLeafNode("operator", expression.arithmeticOperator.type);
    expression.rightHandSide->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ShiftExpression& expression) {
    const std::string nodeId { "shift" };
    openXmlNode(nodeId);
    expression.shiftExpression->accept(*this);
    ident();
    createLeafNode("operator", expression.shiftOperator.type);
    expression.additionExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ComparisonExpression& expression) {
    const std::string nodeId { "comparison" };
    openXmlNode(nodeId);
    expression.leftHandSide->accept(*this);
    ident();
    createLeafNode("operator", expression.comparisonOperator.type);
    expression.rightHandSide->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(BitwiseExpression& expression) {
    const std::string nodeId { "bitwiseExpression" };
    openXmlNode(nodeId);
    expression.leftHandSide->accept(*this);
    ident();
    createLeafNode("operator", expression.bitwiseOperator.type);
    expression.rightHandSide->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(LogicalAndExpression& expression) {
    const std::string nodeId { "logicalAnd" };
    openXmlNode(nodeId);
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(LogicalOrExpression& expression) {
    const std::string nodeId { "logicalOr" };
    openXmlNode(nodeId);
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(AssignmentExpression& expression) {
    const std::string nodeId { "assignment" };
    openXmlNode(nodeId);
    expression.leftHandSide->accept(*this);
    ident();
    createLeafNode("operator", expression.assignmentOperator.type);
    expression.rightHandSide->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ExpressionList& expression) {
    const std::string nodeId { "expressionList" };
    openXmlNode(nodeId);
    expression.leftHandSide->accept(*this);
    expression.rightHandSide->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(JumpStatement& statement) {
    ident();
    createLeafNode("jump", statement.jumpKeyword.type);
}

void SemanticXmlOutputVisitor::visit(ReturnStatement& statement) {
    const std::string nodeId { "return" };
    openXmlNode(nodeId);
    statement.returnExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(IOStatement& statement) {
    openXmlNode(statement.ioKeyword.type);
    statement.expression->accept(*this);
    closeXmlNode(statement.ioKeyword.type);
}

void SemanticXmlOutputVisitor::visit(IfStatement& statement) {
    const std::string nodeId { "if" };
    openXmlNode(nodeId);
    statement.testExpression->accept(*this);
    statement.body->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(IfElseStatement& statement) {
    const std::string nodeId { "if" };
    openXmlNode(nodeId);
    statement.testExpression->accept(*this);
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(LoopStatement& statement) {
    const std::string nodeId { "loop" };
    openXmlNode(nodeId);
    statement.header->accept(*this);
    statement.body->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ForLoopHeader& loopHeader) {
    const std::string nodeId { "forLoopHeader" };
    openXmlNode(nodeId);
    loopHeader.initialization->accept(*this);
    loopHeader.clause->accept(*this);
    loopHeader.increment->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(WhileLoopHeader& loopHeader) {
    const std::string nodeId { "whileLoopHeader" };
    openXmlNode(nodeId);
    loopHeader.clause->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(Pointer& pointer) {
    ident();
    createLeafNode("pointerDereferenceCount", std::to_string(pointer.getDereferenceCount()));
}

void SemanticXmlOutputVisitor::visit(Identifier& identifier) {
    ident();
    createLeafNode("identifier", identifier.getName());
}

void SemanticXmlOutputVisitor::visit(FunctionDeclaration& declaration) {
    const std::string nodeId { "functionDeclaration" };
    openXmlNode(nodeId);
    ident();
    createLeafNode("declaration", declaration.getType(), declaration.getName());
    declaration.parameterList->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ArrayDeclaration& declaration) {
    const std::string nodeId { "arrayDeclaration" };
    openXmlNode(nodeId);
    ident();
    createLeafNode("declaration", declaration.getType(), declaration.getName());
    declaration.subscriptExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ParameterDeclaration& parameter) {
    const std::string nodeId { "parameterDeclaration" };
    openXmlNode(nodeId);
    parameter.type.accept(*this);
    parameter.declaration->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(FunctionDefinition& function) {
    const std::string nodeId { "function" };
    openXmlNode(nodeId);
    function.returnType.accept(*this);
    function.declaration->accept(*this);
    function.body->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(VariableDeclaration& declaration) {
    const std::string nodeId { "varDeclaration" };
    openXmlNode(nodeId);
    declaration.declaredType.accept(*this);
    declaration.declaredVariables->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(VariableDefinition& definition) {
    const std::string nodeId { "varDefinition" };
    openXmlNode(nodeId);
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(Block& block) {
    const std::string nodeId { "block" };
    openXmlNode(nodeId);
    for (const auto& child : block.getChildren()) {
        child->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ListCarrier& listCarrier) {
    for (const auto& child : listCarrier.getChildren()) {
        child->accept(*this);
    }
}

void SemanticXmlOutputVisitor::visit(TranslationUnit& translationUnit) {
    const std::string nodeId { "translationUnit" };
    openXmlNode(nodeId);
    for (const auto& child : translationUnit.getChildren()) {
        child->accept(*this);
    }
    closeXmlNode(nodeId);
}

}
/* namespace semantic_analyzer */
