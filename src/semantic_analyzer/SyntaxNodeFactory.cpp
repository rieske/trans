#include "SyntaxNodeFactory.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>

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
#include "LogicalAndExpression.h"
#include "LogicalOrExpression.h"
#include "LoopStatement.h"
#include "NoArgFunctionCall.h"
#include "NoArgFunctionDeclaration.h"
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
#include "TypeCast.h"
#include "UnaryExpression.h"
#include "VariableDeclaration.h"
#include "WhileLoopHeader.h"

using std::unique_ptr;

using sequence = std::vector<std::string>;

static const std::string UNMATCHED { "<unmatched>" };
static const std::string MATCHED { "<matched>" };
static const std::string DIRECT_DECLARATION { "<dir_decl>" };

namespace semantic_analyzer {

SyntaxNodeFactory::SyntaxNodeFactory() {
	nodeCreatorRegistry[Term::ID][sequence { "(", Expression::ID, ")" }] = SyntaxNodeFactory::parenthesizedExpression;
	nodeCreatorRegistry[Term::ID][sequence { "id" }] = SyntaxNodeFactory::term;
	nodeCreatorRegistry[Term::ID][sequence { "int_const" }] = SyntaxNodeFactory::term;
	nodeCreatorRegistry[Term::ID][sequence { "float_const" }] = SyntaxNodeFactory::term;
	nodeCreatorRegistry[Term::ID][sequence { "literal" }] = SyntaxNodeFactory::term;
	nodeCreatorRegistry[Term::ID][sequence { "string" }] = SyntaxNodeFactory::term;

	nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "[", Expression::ID, "]" }] =
			SyntaxNodeFactory::arrayAccess;
	nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "(", AssignmentExpressionList::ID, ")" }] =
			SyntaxNodeFactory::functionCall;
	nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "(", ")" }] = SyntaxNodeFactory::noargFunctionCall;
	nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "++" }] = SyntaxNodeFactory::postfixIncrementDecrement;
	nodeCreatorRegistry[PostfixExpression::ID][sequence { PostfixExpression::ID, "--" }] = SyntaxNodeFactory::postfixIncrementDecrement;
	nodeCreatorRegistry[PostfixExpression::ID][sequence { Term::ID }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[UnaryExpression::ID][sequence { "++", UnaryExpression::ID }] = SyntaxNodeFactory::prefixIncrementDecrement;
	nodeCreatorRegistry[UnaryExpression::ID][sequence { "--", UnaryExpression::ID }] = SyntaxNodeFactory::prefixIncrementDecrement;
	nodeCreatorRegistry[UnaryExpression::ID][sequence { "<u_op>", TypeCast::ID }] = SyntaxNodeFactory::unaryExpression;
	nodeCreatorRegistry[UnaryExpression::ID][sequence { PostfixExpression::ID }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[TypeCast::ID][sequence { "(", "<type_spec>", ")", TypeCast::ID }] = SyntaxNodeFactory::typeCast;
	nodeCreatorRegistry[TypeCast::ID][sequence { "(", "<type_spec>", Pointer::ID, ")", TypeCast::ID }] = SyntaxNodeFactory::pointerCast;
	nodeCreatorRegistry[TypeCast::ID][sequence { UnaryExpression::ID }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][sequence { ArithmeticExpression::MULTIPLICATION, "<m_op>", TypeCast::ID }] =
			SyntaxNodeFactory::arithmeticExpression;
	nodeCreatorRegistry[ArithmeticExpression::MULTIPLICATION][sequence { TypeCast::ID }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[ArithmeticExpression::ADDITION][sequence { ArithmeticExpression::ADDITION, "<add_op>",
			ArithmeticExpression::MULTIPLICATION }] = SyntaxNodeFactory::arithmeticExpression;
	nodeCreatorRegistry[ArithmeticExpression::ADDITION][sequence { ArithmeticExpression::MULTIPLICATION }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[ShiftExpression::ID][sequence { ShiftExpression::ID, "<s_op>", ArithmeticExpression::ADDITION }] =
			SyntaxNodeFactory::shiftExpression;
	nodeCreatorRegistry[ShiftExpression::ID][sequence { ArithmeticExpression::ADDITION }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[ComparisonExpression::DIFFERENCE][sequence { ComparisonExpression::DIFFERENCE, "<ml_op>", ShiftExpression::ID }] =
			SyntaxNodeFactory::comparisonExpression;
	nodeCreatorRegistry[ComparisonExpression::DIFFERENCE][sequence { ShiftExpression::ID }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[ComparisonExpression::EQUALITY][sequence { ComparisonExpression::EQUALITY, "<eq_op>",
			ComparisonExpression::DIFFERENCE }] = SyntaxNodeFactory::comparisonExpression;
	nodeCreatorRegistry[ComparisonExpression::EQUALITY][sequence { ComparisonExpression::DIFFERENCE }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[BitwiseExpression::AND][sequence { BitwiseExpression::ID, "&", ComparisonExpression::EQUALITY }] =
			SyntaxNodeFactory::bitwiseExpression;
	nodeCreatorRegistry[BitwiseExpression::AND][sequence { ComparisonExpression::EQUALITY }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[BitwiseExpression::XOR][sequence { BitwiseExpression::XOR, "^", BitwiseExpression::AND }] =
			SyntaxNodeFactory::bitwiseExpression;
	nodeCreatorRegistry[BitwiseExpression::XOR][sequence { BitwiseExpression::AND }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[BitwiseExpression::OR][sequence { BitwiseExpression::OR, "|", BitwiseExpression::XOR }] =
			SyntaxNodeFactory::bitwiseExpression;
	nodeCreatorRegistry[BitwiseExpression::OR][sequence { BitwiseExpression::XOR }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[LogicalAndExpression::ID][sequence { LogicalAndExpression::ID, "&&", BitwiseExpression::OR }] =
			SyntaxNodeFactory::logicalAndExpression;
	nodeCreatorRegistry[LogicalAndExpression::ID][sequence { BitwiseExpression::OR }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[LogicalOrExpression::ID][sequence { LogicalOrExpression::ID, "||", Expression::ID }] =
			SyntaxNodeFactory::logicalOrExpression;
	nodeCreatorRegistry[LogicalOrExpression::ID][sequence { LogicalAndExpression::ID }] = SyntaxNodeFactory::backpatchExpression;

	nodeCreatorRegistry[AssignmentExpression::ID][sequence { UnaryExpression::ID, "<a_op>", AssignmentExpression::ID }] =
			SyntaxNodeFactory::assignmentExpression;
	nodeCreatorRegistry[AssignmentExpression::ID][sequence { LogicalOrExpression::ID }] = SyntaxNodeFactory::backpatchExpression;

	nodeCreatorRegistry[AssignmentExpressionList::ID][sequence { AssignmentExpression::ID }] =
			SyntaxNodeFactory::createAssignmentExpressionList;
	nodeCreatorRegistry[AssignmentExpressionList::ID][sequence { AssignmentExpressionList::ID, ",", AssignmentExpression::ID }] =
			SyntaxNodeFactory::addAssignmentExpressionToList;

	nodeCreatorRegistry[Expression::ID][sequence { Expression::ID, ",", AssignmentExpression::ID }] = SyntaxNodeFactory::expressionList;
	nodeCreatorRegistry[Expression::ID][sequence { AssignmentExpression::ID }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[JumpStatement::ID][sequence { "continue", ";" }] = SyntaxNodeFactory::loopJumpStatement;
	nodeCreatorRegistry[JumpStatement::ID][sequence { "break", ";" }] = SyntaxNodeFactory::loopJumpStatement;
	nodeCreatorRegistry[JumpStatement::ID][sequence { "return", Expression::ID, ";" }] = SyntaxNodeFactory::returnStatement;

	nodeCreatorRegistry[IOStatement::ID][sequence { "output", Expression::ID, ";" }] = SyntaxNodeFactory::inputOutputStatement;
	nodeCreatorRegistry[IOStatement::ID][sequence { "input", Expression::ID, ";" }] = SyntaxNodeFactory::inputOutputStatement;

	nodeCreatorRegistry[LoopHeader::ID][sequence { "while", "(", Expression::ID, ")" }] = SyntaxNodeFactory::whileLoopHeader;
	nodeCreatorRegistry[LoopHeader::ID][sequence { "for", "(", Expression::ID, ";", Expression::ID, ";", Expression::ID, ")" }] =
			SyntaxNodeFactory::forLoopHeader;

	nodeCreatorRegistry[UNMATCHED][sequence { "if", "(", Expression::ID, ")", "<stmt>" }] = SyntaxNodeFactory::ifStatement;
	nodeCreatorRegistry[UNMATCHED][sequence { "if", "(", Expression::ID, ")", MATCHED, "else", UNMATCHED }] =
			SyntaxNodeFactory::ifElseStatement;
	nodeCreatorRegistry[UNMATCHED][sequence { LoopHeader::ID, UNMATCHED }] = SyntaxNodeFactory::loopStatement;

	nodeCreatorRegistry[MATCHED][sequence { Expression::ID, ";" }] = SyntaxNodeFactory::expressionStatement;
	nodeCreatorRegistry[MATCHED][sequence { ";" }] = SyntaxNodeFactory::emptyStatement;
	nodeCreatorRegistry[MATCHED][sequence { IOStatement::ID }] = SyntaxNodeFactory::doNothing;
	nodeCreatorRegistry[MATCHED][sequence { Block::ID }] = SyntaxNodeFactory::doNothing;
	nodeCreatorRegistry[MATCHED][sequence { JumpStatement::ID }] = SyntaxNodeFactory::doNothing;
	nodeCreatorRegistry[MATCHED][sequence { "if", "(", Expression::ID, ")", MATCHED, "else", MATCHED }] =
			SyntaxNodeFactory::ifElseStatement;
	nodeCreatorRegistry[MATCHED][sequence { LoopHeader::ID, MATCHED }] = SyntaxNodeFactory::loopStatement;

	nodeCreatorRegistry[ParameterDeclaration::ID][sequence { "<type_spec>", Declaration::ID }] = SyntaxNodeFactory::parameterDeclaration;

	nodeCreatorRegistry[ParameterList::ID][sequence { ParameterDeclaration::ID }] = SyntaxNodeFactory::parameterList;
	nodeCreatorRegistry[ParameterList::ID][sequence { ParameterList::ID, ",", ParameterDeclaration::ID }] =
			SyntaxNodeFactory::addParameterToList;

	nodeCreatorRegistry[DIRECT_DECLARATION][sequence { "(", Declaration::ID, ")" }] = SyntaxNodeFactory::parenthesizedExpression;
	nodeCreatorRegistry[DIRECT_DECLARATION][sequence { "id" }] = SyntaxNodeFactory::identifierDeclaration;
	nodeCreatorRegistry[DIRECT_DECLARATION][sequence { DIRECT_DECLARATION, "(", ParameterList::ID, ")" }] =
			SyntaxNodeFactory::functionDeclaration;
	nodeCreatorRegistry[DIRECT_DECLARATION][sequence { DIRECT_DECLARATION, "[", LogicalOrExpression::ID, "]" }] =
			SyntaxNodeFactory::arrayDeclaration;
	nodeCreatorRegistry[DIRECT_DECLARATION][sequence { DIRECT_DECLARATION, "(", ")" }] = SyntaxNodeFactory::noargFunctionDeclaration;

	nodeCreatorRegistry[Pointer::ID][sequence { "*" }] = SyntaxNodeFactory::pointer;
	nodeCreatorRegistry[Pointer::ID][sequence { Pointer::ID, "*" }] = SyntaxNodeFactory::pointerToPointer;

	nodeCreatorRegistry[Block::ID][sequence { "{", "<var_decls>", "<statements>", "}" }] = SyntaxNodeFactory::doubleBlock;
	nodeCreatorRegistry[Block::ID][sequence { "{", "<var_decls>", "}" }] = SyntaxNodeFactory::singleBlock;
	nodeCreatorRegistry[Block::ID][sequence { "{", "<statements>", "}" }] = SyntaxNodeFactory::singleBlock;
	nodeCreatorRegistry[Block::ID][sequence { "{", "}" }] = SyntaxNodeFactory::parenthesizedExpression;

	nodeCreatorRegistry[Declaration::ID][sequence { Pointer::ID, DIRECT_DECLARATION }] = SyntaxNodeFactory::pointerToDeclaration;
	nodeCreatorRegistry[Declaration::ID][sequence { DIRECT_DECLARATION }] = SyntaxNodeFactory::doNothing;

	nodeCreatorRegistry[DeclarationList::ID][sequence { Declaration::ID }] = SyntaxNodeFactory::declarationList;
	nodeCreatorRegistry[DeclarationList::ID][sequence { DeclarationList::ID, ",", Declaration::ID }] =
			SyntaxNodeFactory::addDeclarationToList;

	nodeCreatorRegistry[VariableDeclaration::ID][sequence { "<type_spec>", DeclarationList::ID, ";" }] =
			SyntaxNodeFactory::variableDeclaration;
	nodeCreatorRegistry[VariableDeclaration::ID][sequence { "<type_spec>", DeclarationList::ID, "=", AssignmentExpression::ID, ";" }] =
			SyntaxNodeFactory::variableDefinition;

	nodeCreatorRegistry[FunctionDefinition::ID][sequence { "<type_spec>", Declaration::ID, Block::ID }] =
			SyntaxNodeFactory::functionDefinition;
}

SyntaxNodeFactory::~SyntaxNodeFactory() {
}

void SyntaxNodeFactory::updateContext(std::string definingSymbol, const std::vector<std::string>& production,
		AbstractSyntaxTreeBuilderContext& context) const {
	try {
		//nodeCreatorRegistry.at(definingSymbol).at(production)(context);
	} catch (std::out_of_range& exception) {
		noCreatorDefined(definingSymbol, production);
	}
}

void SyntaxNodeFactory::noCreatorDefined(std::string definingSymbol, const std::vector<std::string>& production) {
	std::ostringstream productionString;
	for (auto& symbol : production) {
		productionString << symbol << " ";
	}
	throw std::runtime_error { "no AST creator defined for production `" + definingSymbol + " ::= " + productionString.str() + "`" };
}

void SyntaxNodeFactory::doNothing(AbstractSyntaxTreeBuilderContext&) {
}

void SyntaxNodeFactory::parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
}

void SyntaxNodeFactory::term(AbstractSyntaxTreeBuilderContext& context) {
	context.pushExpression(std::unique_ptr<Expression> { new Term(context.popTerminal(), context.scope(), context.line()) });
}

void SyntaxNodeFactory::arrayAccess(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal(); // ]
	context.popTerminal(); // [
	auto subscriptExpression = context.popExpression();
	auto postfixExpression = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new ArrayAccess(std::move(postfixExpression), std::move(subscriptExpression), context.scope(),
					context.line()) });
}

void SyntaxNodeFactory::functionCall(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal(); // )
	context.popTerminal(); // (
	context.pushExpression(
			std::unique_ptr<Expression> { new FunctionCall(context.popExpression(), context.popAssignmentExpressionList(), context.scope(),
					context.line()) });
}

void SyntaxNodeFactory::noargFunctionCall(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal(); // )
	context.popTerminal(); // (
	context.pushExpression(std::unique_ptr<Expression> { new NoArgFunctionCall(context.popExpression(), context.scope(), context.line()) });
}

void SyntaxNodeFactory::postfixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
	context.pushExpression(
			std::unique_ptr<Expression> { new PostfixExpression(context.popExpression(), context.popTerminal(), context.scope()) });
}

void SyntaxNodeFactory::prefixIncrementDecrement(AbstractSyntaxTreeBuilderContext& context) {
	context.pushExpression(
			std::unique_ptr<Expression> { new PrefixExpression(context.popTerminal(), context.popExpression(), context.scope()) });
}

void SyntaxNodeFactory::unaryExpression(AbstractSyntaxTreeBuilderContext& context) {
	context.pushExpression(
			std::unique_ptr<Expression> { new UnaryExpression(context.popTerminal(), context.popExpression(), context.scope()) });
}

void SyntaxNodeFactory::typeCast(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal(); // )
	context.pushExpression(std::unique_ptr<Expression> { new TypeCast(context.popTerminal(), context.popExpression(), context.scope()) });
	context.popTerminal(); // (
}

void SyntaxNodeFactory::pointerCast(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal(); // )
	context.pushExpression(
			std::unique_ptr<Expression> { new PointerCast(context.popTerminal(), context.popPointer(), context.popExpression(),
					context.scope()) });
	context.popTerminal(); // (
}

void SyntaxNodeFactory::arithmeticExpression(AbstractSyntaxTreeBuilderContext& context) {
	auto rightHandSide = context.popExpression();
	auto leftHandSide = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new ArithmeticExpression(std::move(leftHandSide), context.popTerminal(), std::move(rightHandSide),
					context.scope()) });
}

void SyntaxNodeFactory::shiftExpression(AbstractSyntaxTreeBuilderContext& context) {
	auto additionExpression = context.popExpression();
	auto shiftExpression = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new ShiftExpression(std::move(shiftExpression), context.popTerminal(),
					std::move(additionExpression), context.scope()) });
}

void SyntaxNodeFactory::comparisonExpression(AbstractSyntaxTreeBuilderContext& context) {
	auto rightHandSide = context.popExpression();
	auto leftHandSide = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new ComparisonExpression(std::move(leftHandSide), context.popTerminal(), std::move(rightHandSide),
					context.scope()) });
}

void SyntaxNodeFactory::bitwiseExpression(AbstractSyntaxTreeBuilderContext& context) {
	auto rightHandSide = context.popExpression();
	auto leftHandSide = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new BitwiseExpression(std::move(leftHandSide), context.popTerminal(), std::move(rightHandSide),
					context.scope()) });
}

void SyntaxNodeFactory::logicalAndExpression(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	auto rightHandSide = context.popExpression();
	auto leftHandSide = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new LogicalAndExpression(std::move(leftHandSide), std::move(rightHandSide), context.scope(),
					context.line()) });
}

void SyntaxNodeFactory::logicalOrExpression(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	auto rightHandSide = context.popExpression();
	auto leftHandSide = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new LogicalOrExpression(std::move(leftHandSide), std::move(rightHandSide), context.scope(),
					context.line()) });
}

void SyntaxNodeFactory::assignmentExpression(AbstractSyntaxTreeBuilderContext& context) {
	auto rightHandSide = context.popExpression();
	auto leftHandSide = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new AssignmentExpression(std::move(leftHandSide), context.popTerminal(), std::move(rightHandSide),
					context.scope()) });
}

void SyntaxNodeFactory::backpatchExpression(AbstractSyntaxTreeBuilderContext& context) {
	auto backpatchExpression = context.popExpression();
	backpatchExpression->backpatch();
	context.pushExpression(std::move(backpatchExpression));
}

void SyntaxNodeFactory::createAssignmentExpressionList(AbstractSyntaxTreeBuilderContext& context) {
	context.pushAssignmentExpressionList(
			std::unique_ptr<AssignmentExpressionList> { new AssignmentExpressionList(context.popExpression()) });
}

void SyntaxNodeFactory::addAssignmentExpressionToList(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	auto assignmentExpressions = context.popAssignmentExpressionList();
	assignmentExpressions->addExpression(context.popExpression());
	context.pushAssignmentExpressionList(std::move(assignmentExpressions));
}

void SyntaxNodeFactory::expressionList(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	auto rightHandSide = context.popExpression();
	auto leftHandSide = context.popExpression();
	context.pushExpression(
			std::unique_ptr<Expression> { new ExpressionList(std::move(leftHandSide), std::move(rightHandSide), context.scope(),
					context.line()) });
}

void SyntaxNodeFactory::loopJumpStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new JumpStatement(context.popTerminal()) });
	context.popTerminal();
}

void SyntaxNodeFactory::returnStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new ReturnStatement(context.popExpression()) });
}

void SyntaxNodeFactory::inputOutputStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new IOStatement(context.popTerminal(), context.popExpression()) });
}

void SyntaxNodeFactory::whileLoopHeader(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.popTerminal();
	context.pushLoopHeader(std::unique_ptr<LoopHeader> { new WhileLoopHeader(context.popExpression(), context.scope()) });
}

void SyntaxNodeFactory::forLoopHeader(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.popTerminal();
	context.popTerminal();
	context.popTerminal();
	auto increment = context.popExpression();
	auto clause = context.popExpression();
	auto initialization = context.popExpression();
	context.pushLoopHeader(
			std::unique_ptr<LoopHeader> { new ForLoopHeader(std::move(initialization), std::move(clause), std::move(increment),
					context.scope()) });
}

void SyntaxNodeFactory::ifStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.popTerminal();
	context.pushStatement(
			std::unique_ptr<AbstractSyntaxTreeNode> { new IfStatement(context.popExpression(), context.popStatement(), context.scope()) });
}

void SyntaxNodeFactory::ifElseStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.popTerminal();
	context.popTerminal();
	auto falsyStatement = context.popStatement();
	auto truthyStatement = context.popStatement();
	context.pushStatement(
			std::unique_ptr<AbstractSyntaxTreeNode> { new IfElseStatement(context.popExpression(), std::move(truthyStatement),
					std::move(falsyStatement), context.scope()) });
}

void SyntaxNodeFactory::loopStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.pushStatement(
			std::unique_ptr<AbstractSyntaxTreeNode> { new LoopStatement(context.popLoopHeader(), context.popStatement(), context.scope()) });
}

void SyntaxNodeFactory::expressionStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.pushStatement(context.popExpression());
}

void SyntaxNodeFactory::emptyStatement(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
}

void SyntaxNodeFactory::parameterDeclaration(AbstractSyntaxTreeBuilderContext& context) {
	context.pushParameter(
			std::unique_ptr<ParameterDeclaration> { new ParameterDeclaration(context.popTerminal(), context.popDeclaration(),
					context.scope()) });
}

void SyntaxNodeFactory::parameterList(AbstractSyntaxTreeBuilderContext& context) {
	context.pushParameterList(std::unique_ptr<ParameterList> { new ParameterList(context.popParameter()) });
}

void SyntaxNodeFactory::addParameterToList(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	auto parameterList = context.popParameterList();
	parameterList->addParameterDeclaration(context.popParameter());
	context.pushParameterList(std::move(parameterList));
}

void SyntaxNodeFactory::identifierDeclaration(AbstractSyntaxTreeBuilderContext& context) {
	context.pushDeclaration(std::unique_ptr<Declaration> { new Identifier(context.popTerminal()) });
}

// TODO:
void SyntaxNodeFactory::functionDeclaration(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.pushDeclaration(
			std::unique_ptr<Declaration> { new FunctionDeclaration(context.popDeclaration(), context.popParameterList(), context.scope(),
					context.line()) });
	//declaredParams = ((DirectDeclaration *) nonterminalNode)->getParams();
}

void SyntaxNodeFactory::noargFunctionDeclaration(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.pushDeclaration(
			std::unique_ptr<Declaration> { new NoArgFunctionDeclaration(context.popDeclaration(), context.scope(), context.line()) });
}

void SyntaxNodeFactory::arrayDeclaration(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	context.pushDeclaration(
			std::unique_ptr<Declaration> { new ArrayDeclaration(context.popDeclaration(), context.popExpression(), context.scope(),
					context.line()) });
}

void SyntaxNodeFactory::pointer(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.pushPointer(std::unique_ptr<Pointer> { new Pointer() });
}

void SyntaxNodeFactory::pointerToPointer(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	auto pointer = context.popPointer();
	pointer->dereference();
	context.pushPointer(std::move(pointer));
}

void SyntaxNodeFactory::singleBlock(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	//context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new Block( { context.popStatement() }) });
}

void SyntaxNodeFactory::doubleBlock(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	auto secondSubblock = context.popStatement();
	auto firstBlock = context.popStatement();
	//context.pushStatement(std::unique_ptr<AbstractSyntaxTreeNode> { new Block( { std::move(firstBlock), std::move(secondSubblock) }) });
}

void SyntaxNodeFactory::pointerToDeclaration(AbstractSyntaxTreeBuilderContext& context) {
	auto declaration = context.popDeclaration();
	declaration->dereference(context.popPointer()->getType());
	context.pushDeclaration(std::move(declaration));
}

void SyntaxNodeFactory::declarationList(AbstractSyntaxTreeBuilderContext& context) {
	context.pushDeclarationList(std::unique_ptr<DeclarationList> { new DeclarationList(context.popDeclaration()) });
}

void SyntaxNodeFactory::addDeclarationToList(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	auto declarationList = context.popDeclarationList();
	declarationList->addDeclaration(context.popDeclaration());
	context.pushDeclarationList(std::move(declarationList));
}

void SyntaxNodeFactory::variableDeclaration(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	//nonterminalNode = new VariableDeclaration(context.popTerminal(), context.popDeclarationList(), context.scope(), context.line());
}

void SyntaxNodeFactory::variableDefinition(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	//nonterminalNode = new VariableDeclaration(context.popTerminal(), context.popDeclarationList(), context.popExpression(), context.scope(),
	//		context.line());
}

void SyntaxNodeFactory::functionDefinition(AbstractSyntaxTreeBuilderContext& context) {
	//nonterminalNode = new FunctionDefinition(context.popTerminal(), context.popDeclaration(), context.popStatement(), context.scope(),
	//		context.line());
}

} /* namespace semantic_analyzer */
