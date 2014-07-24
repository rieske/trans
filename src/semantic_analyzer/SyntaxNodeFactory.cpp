#include "SyntaxNodeFactory.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "ArithmeticExpression.h"
#include "ArrayAccess.h"
#include "AssignmentExpressionList.h"
#include "BitwiseExpression.h"
#include "ComparisonExpression.h"
#include "FunctionCall.h"
#include "NoArgFunctionCall.h"
#include "Pointer.h"
#include "PointerCast.h"
#include "PostfixExpression.h"
#include "PrefixExpression.h"
#include "ShiftExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"
#include "TypeCast.h"
#include "UnaryExpression.h"

using std::unique_ptr;

using sequence = std::vector<std::string>;

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

} /* namespace semantic_analyzer */
