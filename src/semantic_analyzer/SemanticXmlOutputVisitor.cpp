#include "SemanticXmlOutputVisitor.h"

#include <algorithm>
#include <iterator>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"
#include "BitwiseExpression.h"
#include "ComparisonExpression.h"
#include "Declaration.h"
#include "DeclarationList.h"
#include "FunctionCall.h"
#include "NoArgFunctionCall.h"
#include "ParameterDeclaration.h"
#include "ParameterList.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ShiftExpression.h"
#include "TerminalSymbol.h"
#include "TypeCast.h"
#include "UnaryExpression.h"
#include "Pointer.h"

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

void SemanticXmlOutputVisitor::visit(const AssignmentExpressionList& assignmentExpressions) const {
	openXmlNode(assignmentExpressions.ID);
	for (const auto& assignmentExpression : assignmentExpressions.getAssignmentExpressions()) {
		assignmentExpression->accept(*this);
	}
	closeXmlNode(assignmentExpressions.ID);
}

void SemanticXmlOutputVisitor::visit(const DeclarationList& declarations) const {
	openXmlNode(declarations.ID);
	for (const auto& declaration : declarations.getDeclarations()) {
		declaration->accept(*this);
	}
	closeXmlNode(declarations.ID);
}

void SemanticXmlOutputVisitor::visit(const ArrayAccess& arrayAccess) const {
	const std::string nodeId = "array_access";
	openXmlNode(nodeId);
	arrayAccess.postfixExpression->accept(*this);
	arrayAccess.subscriptExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const FunctionCall& functionCall) const {
	const std::string nodeId = "function_call";
	openXmlNode(nodeId);
	functionCall.postfixExpression->accept(*this);
	functionCall.assignmentExpressionList->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const NoArgFunctionCall& functionCall) const {
	const std::string nodeId = "noarg_function_call";
	openXmlNode(nodeId);
	functionCall.callExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const PostfixExpression& expression) const {
	const std::string nodeId = "postfix_expression";
	openXmlNode(nodeId);
	expression.postfixExpression->accept(*this);
	createLeafNode("operator", expression.postfixOperator.type);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const PrefixExpression& expression) const {
	const std::string nodeId = "prefix_expression";
	openXmlNode(nodeId);
	createLeafNode("operator", expression.incrementOperator.type);
	expression.unaryExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const UnaryExpression& expression) const {
	const std::string nodeId = "unary_expression";
	openXmlNode(nodeId);
	createLeafNode("operator", expression.unaryOperator.type);
	expression.castExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const TypeCast& expression) const {
	const std::string nodeId = "type_cast";
	openXmlNode(nodeId);
	createLeafNode("typeSpecifier", expression.typeSpecifier.type);
	expression.castExpression->accept(*this);
	closeXmlNode(nodeId);
}

void SemanticXmlOutputVisitor::visit(const PointerCast& expression) const {
	const std::string nodeId = "pointer_cast";
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

} /* namespace semantic_analyzer */
