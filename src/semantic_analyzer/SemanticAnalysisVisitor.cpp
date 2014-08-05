#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

SemanticAnalysisVisitor::SemanticAnalysisVisitor() {
}

SemanticAnalysisVisitor::~SemanticAnalysisVisitor() {
}

void SemanticAnalysisVisitor::visit(const ParameterList& parameterList) const {
}

void SemanticAnalysisVisitor::visit(const AssignmentExpressionList& expressions) const {
}

void SemanticAnalysisVisitor::visit(const DeclarationList& declarations) const {
}

void SemanticAnalysisVisitor::visit(const ArrayAccess& arrayAccess) const {
}

void SemanticAnalysisVisitor::visit(const FunctionCall& functionCall) const {
}

void SemanticAnalysisVisitor::visit(const NoArgFunctionCall& functionCall) const {
}

void SemanticAnalysisVisitor::visit(const Term& term) const {
}

void SemanticAnalysisVisitor::visit(const PostfixExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const PrefixExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const UnaryExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const TypeCast& expression) const {
}

void SemanticAnalysisVisitor::visit(const PointerCast& expression) const {
}

void SemanticAnalysisVisitor::visit(const ArithmeticExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const ShiftExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const ComparisonExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const BitwiseExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const LogicalAndExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const LogicalOrExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const AssignmentExpression& expression) const {
}

void SemanticAnalysisVisitor::visit(const ExpressionList& expression) const {
}

void SemanticAnalysisVisitor::visit(const JumpStatement& statement) const {
}

void SemanticAnalysisVisitor::visit(const ReturnStatement& statement) const {
}

void SemanticAnalysisVisitor::visit(const IOStatement& statement) const {
}

void SemanticAnalysisVisitor::visit(const IfStatement& statement) const {
}

void SemanticAnalysisVisitor::visit(const IfElseStatement& statement) const {
}

void SemanticAnalysisVisitor::visit(const LoopStatement& statement) const {
}

void SemanticAnalysisVisitor::visit(const ForLoopHeader& loopHeader) const {
}

void SemanticAnalysisVisitor::visit(const WhileLoopHeader& loopHeader) const {
}

void SemanticAnalysisVisitor::visit(const Pointer& pointer) const {
}

void SemanticAnalysisVisitor::visit(const Identifier& identifier) const {
}

void SemanticAnalysisVisitor::visit(const FunctionDeclaration& declaration) const {
}

void SemanticAnalysisVisitor::visit(const ArrayDeclaration& declaration) const {
}

void SemanticAnalysisVisitor::visit(const ParameterDeclaration& parameter) const {
}

void SemanticAnalysisVisitor::visit(const FunctionDefinition& function) const {
}

void SemanticAnalysisVisitor::visit(const VariableDeclaration& declaration) const {
}

void SemanticAnalysisVisitor::visit(const VariableDefinition& definition) const {
}

void SemanticAnalysisVisitor::visit(const Block& block) const {
}

void SemanticAnalysisVisitor::visit(const ListCarrier& listCarrier) const {
}

void SemanticAnalysisVisitor::visit(const TranslationUnit& translationUnit) const {
}

} /* namespace semantic_analyzer */
