#include "SemanticXmlOutputVisitor.h"

#include <algorithm>
#include <iterator>
#include <memory>

#include "../ast/ArrayDeclaration.h"
#include "../ast/AssignmentExpressionList.h"
#include "../ast/Block.h"
#include "../ast/DeclarationList.h"
#include "../ast/DoubleOperandExpression.h"
#include "../ast/ForLoopHeader.h"
#include "../ast/FunctionCall.h"
#include "../ast/FunctionDeclaration.h"
#include "../ast/FunctionDefinition.h"
#include "../ast/Identifier.h"
#include "../ast/IfElseStatement.h"
#include "../ast/IfStatement.h"
#include "../ast/IOStatement.h"
#include "../ast/JumpStatement.h"
#include "../ast/ListCarrier.h"
#include "../ast/LoopStatement.h"
#include "../ast/Operator.h"
#include "../ast/ParameterDeclaration.h"
#include "../ast/ParameterList.h"
#include "../ast/Pointer.h"
#include "../ast/PointerCast.h"
#include "../ast/PostfixExpression.h"
#include "../ast/ReturnStatement.h"
#include "../ast/Term.h"
#include "../ast/TerminalSymbol.h"
#include "../ast/TranslationUnit.h"
#include "../ast/TypeCast.h"
#include "../ast/TypeSpecifier.h"
#include "../ast/VariableDeclaration.h"
#include "../ast/VariableDefinition.h"
#include "../ast/WhileLoopHeader.h"
#include "ast/ExpressionList.h"
#include "ast/ArrayAccess.h"
#include "ast/PrefixExpression.h"
#include "ast/UnaryExpression.h"
#include "ast/ArithmeticExpression.h"
#include "ast/ShiftExpression.h"
#include "ast/ComparisonExpression.h"
#include "ast/BitwiseExpression.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"

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

void SemanticXmlOutputVisitor::createLeafNode(std::string nodeName, int dereferenceCount, std::string value) const {
    *outputStream << "<" << nodeName << " dereferenceCount='" << std::to_string(dereferenceCount) << "'>" << value << "</" << nodeName << ">\n";
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

void SemanticXmlOutputVisitor::visit(ast::TypeSpecifier& typeSpecifier) {
    ident();
    createLeafNode("typeSpecifier", typeSpecifier.getName());
}

void SemanticXmlOutputVisitor::visit(ast::ParameterList& parameterList) {
    const std::string nodeId { "parameterList" };
    openXmlNode(nodeId);
    for (const auto& parameter : parameterList.getDeclaredParameters()) {
        parameter->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::AssignmentExpressionList& expressions) {
    const std::string nodeId { "assignmentExpressions" };
    openXmlNode(nodeId);
    for (const auto& expression : expressions.getExpressions()) {
        expression->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::DeclarationList& declarations) {
    const std::string nodeId { "declarations" };
    openXmlNode(nodeId);
    for (const auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ArrayAccess& arrayAccess) {
    const std::string nodeId { "arrayAccess" };
    openXmlNode(nodeId);
    arrayAccess.getLeftOperand()->accept(*this);
    arrayAccess.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::FunctionCall& functionCall) {
    const std::string nodeId { "functionCall" };
    openXmlNode(nodeId);
    functionCall.getOperand()->accept(*this);
    functionCall.getArgumentList()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::Term& term) {
    ident();
    createLeafNode("term", term.getTypeSymbol(), term.getValue());
}

void SemanticXmlOutputVisitor::visit(ast::PostfixExpression& expression) {
    const std::string nodeId { "postfixExpression" };
    openXmlNode(nodeId);
    expression.getOperand()->accept(*this);
    expression.getOperator()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::PrefixExpression& expression) {
    const std::string nodeId { "prefixExpression" };
    openXmlNode(nodeId);
    expression.getOperator()->accept(*this);
    expression.getOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::UnaryExpression& expression) {
    const std::string nodeId { "unaryExpression" };
    openXmlNode(nodeId);
    expression.getOperator()->accept(*this);
    expression.getOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::TypeCast& expression) {
    const std::string nodeId { "typeCast" };
    openXmlNode(nodeId);
    expression.getType().accept(*this);
    expression.getOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::PointerCast& expression) {
    const std::string nodeId { "pointerCast" };
    openXmlNode(nodeId);
    expression.getType().accept(*this);
    expression.getPointer()->accept(*this);
    expression.getOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ArithmeticExpression& expression) {
    const std::string nodeId { "arithmeticExpression" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getOperator()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ShiftExpression& expression) {
    const std::string nodeId { "shift" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getOperator()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ComparisonExpression& expression) {
    const std::string nodeId { "comparison" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getOperator()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::BitwiseExpression& expression) {
    const std::string nodeId { "bitwiseExpression" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getOperator()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::LogicalAndExpression& expression) {
    const std::string nodeId { "logicalAnd" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::LogicalOrExpression& expression) {
    const std::string nodeId { "logicalOr" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::AssignmentExpression& expression) {
    const std::string nodeId { "assignment" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getOperator()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ExpressionList& expression) {
    const std::string nodeId { "expressionList" };
    openXmlNode(nodeId);
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::Operator& op) {
    ident();
    createLeafNode("operator", op.getLexeme());
}

void SemanticXmlOutputVisitor::visit(ast::JumpStatement& statement) {
    ident();
    createLeafNode("jump", statement.jumpKeyword.type);
}

void SemanticXmlOutputVisitor::visit(ast::ReturnStatement& statement) {
    const std::string nodeId { "return" };
    openXmlNode(nodeId);
    statement.returnExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::IOStatement& statement) {
    openXmlNode(statement.ioKeyword.type);
    statement.expression->accept(*this);
    closeXmlNode(statement.ioKeyword.type);
}

void SemanticXmlOutputVisitor::visit(ast::IfStatement& statement) {
    const std::string nodeId { "if" };
    openXmlNode(nodeId);
    statement.testExpression->accept(*this);
    statement.body->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::IfElseStatement& statement) {
    const std::string nodeId { "if" };
    openXmlNode(nodeId);
    statement.testExpression->accept(*this);
    statement.truthyBody->accept(*this);
    statement.falsyBody->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::LoopStatement& statement) {
    const std::string nodeId { "loop" };
    openXmlNode(nodeId);
    statement.header->accept(*this);
    statement.body->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ForLoopHeader& loopHeader) {
    const std::string nodeId { "forLoopHeader" };
    openXmlNode(nodeId);
    loopHeader.initialization->accept(*this);
    loopHeader.clause->accept(*this);
    loopHeader.increment->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    const std::string nodeId { "whileLoopHeader" };
    openXmlNode(nodeId);
    loopHeader.clause->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::Pointer& pointer) {
    ident();
    createLeafNode("dereferenceCount", std::to_string(pointer.getDereferenceCount()));
}

void SemanticXmlOutputVisitor::visit(ast::Identifier& identifier) {
    ident();
    createLeafNode("identifier", identifier.getName());
}

void SemanticXmlOutputVisitor::visit(ast::FunctionDeclaration& declaration) {
    const std::string nodeId { "functionDeclaration" };
    openXmlNode(nodeId);
    ident();
    createLeafNode("declaration", declaration.getDereferenceCount(), declaration.getName());
    declaration.parameterList->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ArrayDeclaration& declaration) {
    const std::string nodeId { "arrayDeclaration" };
    openXmlNode(nodeId);
    ident();
    createLeafNode("declaration", declaration.getDereferenceCount(), declaration.getName());
    declaration.subscriptExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ParameterDeclaration& parameter) {
    const std::string nodeId { "parameterDeclaration" };
    openXmlNode(nodeId);
    parameter.type.accept(*this);
    parameter.declaration->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::FunctionDefinition& function) {
    const std::string nodeId { "function" };
    openXmlNode(nodeId);
    function.returnType.accept(*this);
    function.declaration->accept(*this);
    function.body->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::VariableDeclaration& declaration) {
    const std::string nodeId { "varDeclaration" };
    openXmlNode(nodeId);
    declaration.declaredType.accept(*this);
    declaration.declaredVariables->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::VariableDefinition& definition) {
    const std::string nodeId { "varDefinition" };
    openXmlNode(nodeId);
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::Block& block) {
    const std::string nodeId { "block" };
    openXmlNode(nodeId);
    for (const auto& child : block.getChildren()) {
        child->accept(*this);
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ListCarrier& listCarrier) {
    for (const auto& child : listCarrier.getChildren()) {
        child->accept(*this);
    }
}

void SemanticXmlOutputVisitor::visit(ast::TranslationUnit& translationUnit) {
    const std::string nodeId { "translationUnit" };
    openXmlNode(nodeId);
    for (const auto& child : translationUnit.getChildren()) {
        child->accept(*this);
    }
    closeXmlNode(nodeId);
}

}
/* namespace semantic_analyzer */
