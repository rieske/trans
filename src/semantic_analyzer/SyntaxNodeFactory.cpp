#include "SyntaxNodeFactory.h"

#include <memory>
#include <sstream>
#include <stdexcept>

#include "AssignmentExpressionList.h"
#include "PostfixExpression.h"
#include "Term.h"
#include "TerminalSymbol.h"

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

void SyntaxNodeFactory::parenthesizedExpression(AbstractSyntaxTreeBuilderContext& context) {
	//context.popTerminal();
	//context.popTerminal();
}

void SyntaxNodeFactory::term(AbstractSyntaxTreeBuilderContext& context) {
	//context.pushExpression(std::unique_ptr<Expression> {new Term(context.popTerminal(), currentScope, currentLine)});
}

void SyntaxNodeFactory::arrayAccess(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	auto subscriptExpression = context.popExpression();
	auto postfixExpression = context.popExpression();
	/*context.pushExpression(
	 std::unique_ptr<Expression> { new PostfixExpression(std::move(postfixExpression), std::move(subscriptExpression), currentScope,
	 currentLine) });*/
}

void SyntaxNodeFactory::functionCall(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	/*context.pushExpression(
	 std::unique_ptr<Expression> { new PostfixExpression(context.popExpression(), context.popAssignmentExpressionList(),
	 currentScope, currentLine) });*/
}

void SyntaxNodeFactory::noargFunctionCall(AbstractSyntaxTreeBuilderContext& context) {
	context.popTerminal();
	context.popTerminal();
	//context.pushExpression(std::unique_ptr<Expression> { new PostfixExpression(context.popExpression(), currentScope, currentLine) });
}

/*
 * if (definingSymbol == PostfixExpression::ID) {
 if (production.produces( { PostfixExpression::ID, "[", Expression::ID, "]" })) {

 } else if (production.produces( { PostfixExpression::ID, "(", AssignmentExpressionList::ID, ")" })) {

 } else if (production.produces( { PostfixExpression::ID, "(", ")" })) {

 */

} /* namespace semantic_analyzer */
