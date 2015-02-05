#include "SemanticXmlOutputVisitor.h"

#include <algorithm>
#include <iterator>
#include <memory>

#include "../ast/ArrayDeclarator.h"
#include "../ast/Block.h"
#include "../ast/DoubleOperandExpression.h"
#include "../ast/ForLoopHeader.h"
#include "../ast/FunctionCall.h"
#include "../ast/FunctionDeclarator.h"
#include "../ast/FunctionDefinition.h"
#include "../ast/Identifier.h"
#include "../ast/IfElseStatement.h"
#include "../ast/IfStatement.h"
#include "../ast/IOStatement.h"
#include "../ast/JumpStatement.h"
#include "../ast/LoopStatement.h"
#include "../ast/Operator.h"
#include "../ast/FormalArgument.h"
#include "../ast/Pointer.h"
#include "../ast/PostfixExpression.h"
#include "../ast/ReturnStatement.h"
#include "../ast/IdentifierExpression.h"
#include "../ast/ConstantExpression.h"
#include "../ast/TerminalSymbol.h"
#include "../ast/TypeCast.h"
#include "../ast/TypeSpecifier.h"
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
#include "ast/AssignmentExpression.h"
#include "ast/Declarator.h"
#include "ast/Declaration.h"
#include "ast/InitializedDeclarator.h"

static const std::string IDENTATION { "  " };

namespace {

std::string to_string(const TypeQualifier& qualifier) {
    switch (qualifier) {
    case TypeQualifier::CONST:
        return "const";
    case TypeQualifier::VOLATILE:
        return "volatile";
    default:
        throw std::runtime_error { "unrecognized TypeQualifier in SemanticXmlOutputVisitor" };
    }
}

}

namespace semantic_analyzer {

SemanticXmlOutputVisitor::SemanticXmlOutputVisitor(std::ostream* outputStream) :
        outputStream { outputStream }
{
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

void SemanticXmlOutputVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    const std::string nodeId { "declarationSpecifiers" };
    openXmlNode(nodeId);
    for (auto& typeSpecifier : declarationSpecifiers.getTypeSpecifiers()) {
        ident();
        createLeafNode("typeSpecifier", typeSpecifier.getName());
    }
    for (auto& typeQualifier : declarationSpecifiers.getTypeQualifiers()) {
        ident();
        createLeafNode("typeQualifier", to_string(typeQualifier));
    }
    for (auto& storageSpecifier : declarationSpecifiers.getStorageSpecifiers()) {
        ident();
        createLeafNode("storageSpecifier", to_string(storageSpecifier));
    }
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::Declaration& declaration) {
    const std::string nodeId { "declaration" };
    openXmlNode(nodeId);
    declaration.visitChildren(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::Declarator& declarator) {
    const std::string nodeId { "declarator" };
    openXmlNode(nodeId);
    declarator.visitChildren(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::InitializedDeclarator& declarator) {
    const std::string nodeId { "initDeclarator" };
    openXmlNode(nodeId);
    declarator.visitChildren(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ArrayAccess& arrayAccess) {
    const std::string nodeId { "arrayAccess" };
    openXmlNode(nodeId);
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::FunctionCall& functionCall) {
    const std::string nodeId { "functionCall" };
    openXmlNode(nodeId);
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::IdentifierExpression& identifier) {
    ident();
    createLeafNode("identifier", identifier.getIdentifier());
}

void SemanticXmlOutputVisitor::visit(ast::ConstantExpression& constant) {
    ident();
    createLeafNode("constant", constant.getValue());
}

void SemanticXmlOutputVisitor::visit(ast::PostfixExpression& expression) {
    const std::string nodeId { "postfixExpression" };
    openXmlNode(nodeId);
    expression.visitOperand(*this);
    expression.getOperator()->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::PrefixExpression& expression) {
    const std::string nodeId { "prefixExpression" };
    openXmlNode(nodeId);
    expression.getOperator()->accept(*this);
    expression.visitOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::UnaryExpression& expression) {
    const std::string nodeId { "unaryExpression" };
    openXmlNode(nodeId);
    expression.getOperator()->accept(*this);
    expression.visitOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::TypeCast& expression) {
    const std::string nodeId { "typeCast" };
    openXmlNode(nodeId);
    ident();
    createLeafNode("typeSpecifier", expression.getType().getName());
    expression.visitOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ArithmeticExpression& expression) {
    const std::string nodeId { "arithmeticExpression" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.getOperator()->accept(*this);
    expression.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ShiftExpression& expression) {
    const std::string nodeId { "shift" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.getOperator()->accept(*this);
    expression.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ComparisonExpression& expression) {
    const std::string nodeId { "comparison" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.getOperator()->accept(*this);
    expression.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::BitwiseExpression& expression) {
    const std::string nodeId { "bitwiseExpression" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.getOperator()->accept(*this);
    expression.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::LogicalAndExpression& expression) {
    const std::string nodeId { "logicalAnd" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::LogicalOrExpression& expression) {
    const std::string nodeId { "logicalOr" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::AssignmentExpression& expression) {
    const std::string nodeId { "assignment" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.getOperator()->accept(*this);
    expression.visitRightOperand(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ExpressionList& expression) {
    const std::string nodeId { "expressionList" };
    openXmlNode(nodeId);
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
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

void SemanticXmlOutputVisitor::visit(ast::FunctionDeclarator& declarator) {
    const std::string nodeId { "functionDeclarator" };
    openXmlNode(nodeId);
    ident();
    createLeafNode("declarator", declarator.getName());
    declarator.visitFormalArguments(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::ArrayDeclarator& declaration) {
    const std::string nodeId { "arrayDeclaration" };
    openXmlNode(nodeId);
    ident();
    createLeafNode("declaration", declaration.getName());
    declaration.subscriptExpression->accept(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::FormalArgument& parameter) {
    const std::string nodeId { "formalArgument" };
    openXmlNode(nodeId);
    parameter.visitSpecifiers(*this);
    parameter.visitDeclarator(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::FunctionDefinition& function) {
    const std::string nodeId { "function" };
    openXmlNode(nodeId);
    function.visitReturnType(*this);
    function.visitDeclarator(*this);
    function.visitBody(*this);
    closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(ast::Block& block) {
    const std::string nodeId { "block" };
    openXmlNode(nodeId);
    block.visitChildren(*this);
    closeXmlNode(nodeId);
}

}
/* namespace semantic_analyzer */
