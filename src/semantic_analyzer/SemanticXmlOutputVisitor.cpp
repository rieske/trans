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
#include "TerminalSymbol.h"
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

void SemanticXmlOutputVisitor::openXmlNode(const std::string& nodeName) const {
	ident();
	*outputStream << "<" << stripLabel(nodeName) << ">\n";
	++identation;
}

void SemanticXmlOutputVisitor::closeXmlNode(const std::string& nodeName) const {
	--identation;
	ident();
	*outputStream << "</" << stripLabel(nodeName) << ">\n";
}

void SemanticXmlOutputVisitor::createLeafNode(std::string nodeName, std::string value) const {
	*outputStream << "<" << nodeName << ">" << value << "</" << nodeName << ">\n";
}

std::string SemanticXmlOutputVisitor::stripLabel(std::string label) const {
	label.erase(std::remove(label.begin(), label.end(), '<'), label.end());
	label.erase(std::remove(label.begin(), label.end(), '>'), label.end());
	return label;
}

void SemanticXmlOutputVisitor::ident() const {
	for (int i = 0; i != identation; ++i) {
		*outputStream << IDENTATION;
	}
}

void SemanticXmlOutputVisitor::visit(const AbstractSyntaxTreeNode& node) const {

}

void SemanticXmlOutputVisitor::visit(const NonterminalNode& node) const {
	openXmlNode(node.typeId());
	for (const auto& child : node.getChildren()) {
		child->accept(*this);
	}
	closeXmlNode(node.typeId());
}

void SemanticXmlOutputVisitor::visit(const ParameterList& parameterList) const {
	openXmlNode(parameterList.ID);
	for (const auto& parameter : parameterList.getDeclaredParameters()) {
		parameter->accept(*this);
	}
	closeXmlNode(parameterList.ID);
}

void SemanticXmlOutputVisitor::visit(const AssignmentExpressionList& expressions) const {
	const std::string nodeId = "assignmentExpressions";
	openXmlNode(nodeId);
	for (const auto& expression : expressions.getExpressions()) {
		expression->accept(*this);
	}
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const DeclarationList& declarations) const {
	openXmlNode(declarations.ID);
	for (const auto& declaration : declarations.getDeclarations()) {
		declaration->accept(*this);
	}
	closeXmlNode(declarations.ID);
}

void SemanticXmlOutputVisitor::visit(const ArrayAccess& arrayAccess) const {
	const std::string nodeId = "arrayAccess";
	openXmlNode(nodeId);
	arrayAccess.postfixExpression->accept(*this);
	arrayAccess.subscriptExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const FunctionCall& functionCall) const {
	const std::string nodeId = "functionCall";
	openXmlNode(nodeId);
	functionCall.postfixExpression->accept(*this);
	functionCall.assignmentExpressionList->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const NoArgFunctionCall& functionCall) const {
	const std::string nodeId = "noargFunctionCall";
	openXmlNode(nodeId);
	functionCall.callExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const PostfixExpression& expression) const {
	const std::string nodeId = "postfixExpression";
	openXmlNode(nodeId);
	expression.postfixExpression->accept(*this);
	createLeafNode("operator", expression.postfixOperator.type);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const PrefixExpression& expression) const {
	const std::string nodeId = "prefixExpression";
	openXmlNode(nodeId);
	createLeafNode("operator", expression.incrementOperator.type);
	expression.unaryExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const UnaryExpression& expression) const {
	const std::string nodeId = "unaryExpression";
	openXmlNode(nodeId);
	createLeafNode("operator", expression.unaryOperator.type);
	expression.castExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const TypeCast& expression) const {
	const std::string nodeId = "typeCast";
	openXmlNode(nodeId);
	createLeafNode("typeSpecifier", expression.typeSpecifier.type);
	expression.castExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const PointerCast& expression) const {
	const std::string nodeId = "pointerCast";
	openXmlNode(nodeId);
	createLeafNode("typeSpecifier", expression.typeSpecifier.type);
	expression.pointer->accept(*this);
	expression.castExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ArithmeticExpression& expression) const {
	const std::string nodeId = "arithmeticExpression";
	openXmlNode(nodeId);
	expression.leftHandSide->accept(*this);
	ident();
	createLeafNode("operator", expression.arithmeticOperator.type);
	expression.rightHandSide->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ShiftExpression& expression) const {
	const std::string nodeId = "shift";
	openXmlNode(nodeId);
	expression.shiftExpression->accept(*this);
	ident();
	createLeafNode("operator", expression.shiftOperator.type);
	expression.additionExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ComparisonExpression& expression) const {
	const std::string nodeId = "comparison";
	openXmlNode(nodeId);
	expression.leftHandSide->accept(*this);
	ident();
	createLeafNode("operator", expression.comparisonOperator.type);
	expression.rightHandSide->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const BitwiseExpression& expression) const {
	const std::string nodeId = "bitwiseExpression";
	openXmlNode(nodeId);
	expression.leftHandSide->accept(*this);
	ident();
	createLeafNode("operator", expression.bitwiseOperator.type);
	expression.rightHandSide->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const LogicalAndExpression& expression) const {
	const std::string nodeId = "logicalAnd";
	openXmlNode(nodeId);
	expression.leftHandSide->accept(*this);
	expression.rightHandSide->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const LogicalOrExpression& expression) const {
	const std::string nodeId = "logicalOr";
	openXmlNode(nodeId);
	expression.leftHandSide->accept(*this);
	expression.rightHandSide->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const AssignmentExpression& expression) const {
	const std::string nodeId = "assignment";
	openXmlNode(nodeId);
	expression.leftHandSide->accept(*this);
	ident();
	createLeafNode("operator", expression.assignmentOperator.type);
	expression.rightHandSide->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ExpressionList& expression) const {
	const std::string nodeId = "expressionList";
	openXmlNode(nodeId);
	expression.leftHandSide->accept(*this);
	expression.rightHandSide->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const JumpStatement& statement) const {
	ident();
	createLeafNode("jump", statement.jumpKeyword.type);
}

void SemanticXmlOutputVisitor::visit(const ReturnStatement& statement) const {
	const std::string nodeId = "return";
	openXmlNode(nodeId);
	statement.returnExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const IOStatement& statement) const {
	openXmlNode(statement.ioKeyword.type);
	statement.expression->accept(*this);
	closeXmlNode(statement.ioKeyword.type);
}

void SemanticXmlOutputVisitor::visit(const IfStatement& statement) const {
	const std::string nodeId = "if";
	openXmlNode(nodeId);
	statement.testExpression->accept(*this);
	statement.body->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const IfElseStatement& statement) const {
	const std::string nodeId = "if";
	openXmlNode(nodeId);
	statement.testExpression->accept(*this);
	statement.truthyBody->accept(*this);
	statement.falsyBody->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const LoopStatement& statement) const {
	const std::string nodeId = "loop";
	openXmlNode(nodeId);
	statement.header->accept(*this);
	statement.body->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ForLoopHeader& loopHeader) const {
	const std::string nodeId = "forLoopHeader";
	openXmlNode(nodeId);
	loopHeader.initialization->accept(*this);
	loopHeader.clause->accept(*this);
	loopHeader.increment->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const WhileLoopHeader& loopHeader) const {
	const std::string nodeId = "whileLoopHeader";
	openXmlNode(nodeId);
	loopHeader.clause->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const Pointer& pointer) const {
	ident();
	createLeafNode("pointer", pointer.getType());
}

void SemanticXmlOutputVisitor::visit(const Identifier& identifier) const {
	ident();
	createLeafNode("identifier", identifier.identifier);
}

void SemanticXmlOutputVisitor::visit(const FunctionDeclaration& declaration) const {
	const std::string nodeId = "functionDeclaration";
	openXmlNode(nodeId);
	declaration.declaration->accept(*this);
	declaration.parameterList->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ArrayDeclaration& declaration) const {
	const std::string nodeId = "arrayDeclaration";
	openXmlNode(nodeId);
	declaration.declaration->accept(*this);
	declaration.subscriptExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ParameterDeclaration& parameter) const {
	const std::string nodeId = "parameterDeclaration";
	openXmlNode(nodeId);
	ident();
	createLeafNode("type", parameter.getBasicType());
	parameter.declaration->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const FunctionDefinition& function) const {
	const std::string nodeId = "function";
	openXmlNode(nodeId);
	ident();
	createLeafNode("type", function.typeSpecifier.value);
	function.declaration->accept(*this);
	function.body->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const VariableDeclaration& declaration) const {
	const std::string nodeId = "varDeclaration";
	openXmlNode(nodeId);
	ident();
	createLeafNode("type", declaration.typeSpecifier.value);
	declaration.declarationList->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const VariableDefinition& definition) const {
	const std::string nodeId = "varDefinition";
	openXmlNode(nodeId);
	definition.declaration->accept(*this);
	definition.initializerExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const Block& block) const {
	const std::string nodeId = "block";
	openXmlNode(nodeId);
	for (const auto& child : block.getChildren()) {
		child->accept(*this);
	}
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const ListCarrier& listCarrier) const {
	for (const auto& child : listCarrier.getChildren()) {
		child->accept(*this);
	}
}

}
/* namespace semantic_analyzer */
