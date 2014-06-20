#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileGrammar.h"
#include "parser/GrammarSymbol.h"
#include "parser/Grammar.h"
#include "parser/FirstTable.h"

using namespace testing;
using std::unique_ptr;
using std::vector;

TEST(FirstTable, computesFirstTableForGrammarRules) {
	BNFFileGrammar grammar { "resources/configuration/grammar.bnf" };

	FirstTable first { grammar.getNonterminals() };

	auto first0 = first(grammar.getNonterminals().at(0));
	ASSERT_THAT(first0, SizeIs(4));
	ASSERT_THAT(first0.at(0)->getName(), Eq("int"));
	ASSERT_THAT(first0.at(1)->getName(), Eq("char"));
	ASSERT_THAT(first0.at(2)->getName(), Eq("void"));
	ASSERT_THAT(first0.at(3)->getName(), Eq("float"));

	/*
	 * FIRST(<program>): 'int' 'char' 'void' 'float'
	 FIRST(<func_decls>): 'int' 'char' 'void' 'float'
	 FIRST(<var_decls>): 'int' 'char' 'void' 'float'
	 FIRST(<func_decl>): 'int' 'char' 'void' 'float'
	 FIRST(<var_decl>): 'int' 'char' 'void' 'float'
	 FIRST(<type_spec>): 'int' 'char' 'void' 'float'
	 FIRST(<decl>): '*' '(' 'id'
	 FIRST(<block>): '{'
	 FIRST(<decls>): '*' '(' 'id'
	 FIRST(<a_expr>): '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<ptr>): '*'
	 FIRST(<dir_decl>): '(' 'id'
	 FIRST(<statements>): '{' ';' 'if' 'output' 'input' 'continue' 'break' 'return' 'while' 'for' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<param_list>): 'int' 'char' 'void' 'float'
	 FIRST(<log_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<param_decl>): 'int' 'char' 'void' 'float'
	 FIRST(<stmt>): '{' ';' 'if' 'output' 'input' 'continue' 'break' 'return' 'while' 'for' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<matched>): '{' ';' 'if' 'output' 'input' 'continue' 'break' 'return' 'while' 'for' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<unmatched>): 'if' 'while' 'for'
	 FIRST(<expr>): '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<io_stmt>): 'output' 'input'
	 FIRST(<jmp_stmt>): 'continue' 'break' 'return'
	 FIRST(<loop_hdr>): 'while' 'for'
	 FIRST(<u_expr>): '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<a_op>): '=' '+=' '-=' '*=' '/=' '%=' '&=' '^=' '|=' '<<=' '>>='
	 FIRST(<a_expressions>): '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<log_and_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<or_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<xor_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<and_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<eq_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<eq_op>): '==' '!='
	 FIRST(<ml_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<ml_op>): '>' '<' '>=' '<='
	 FIRST(<s_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<s_op>): '>>' '<<'
	 FIRST(<add_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<add_op>): '+' '-'
	 FIRST(<factor>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<m_op>): '*' '/' '%'
	 FIRST(<cast_expr>): '(' '++' '--' '&' '*' '+' '-' '!' 'id' 'int_const' 'float_const' 'literal' 'string'
	 FIRST(<u_op>): '&' '*' '+' '-' '!'
	 FIRST(<postfix_expr>): 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 FIRST(<term>): 'id' 'int_const' 'float_const' 'literal' 'string' '('
	 */
}

TEST(FirstTable, computesFirstTableForSimpleGrammarRules) {

	unique_ptr<GrammarSymbol> expression = unique_ptr<GrammarSymbol> { new GrammarSymbol("<expr>", 0) };
	unique_ptr<GrammarSymbol> term = unique_ptr<GrammarSymbol> { new GrammarSymbol("<term>", 0) };
	unique_ptr<GrammarSymbol> factor = unique_ptr<GrammarSymbol> { new GrammarSymbol("<factor>", 0) };
	unique_ptr<GrammarSymbol> operand = unique_ptr<GrammarSymbol> { new GrammarSymbol("<operand>", 0) };
	unique_ptr<GrammarSymbol> identifier = unique_ptr<GrammarSymbol> { new GrammarSymbol("identifier", 0) };
	unique_ptr<GrammarSymbol> constant = unique_ptr<GrammarSymbol> { new GrammarSymbol("constant", 0) };
	unique_ptr<GrammarSymbol> addOper = unique_ptr<GrammarSymbol> { new GrammarSymbol("+", 0) };
	unique_ptr<GrammarSymbol> multiOper = unique_ptr<GrammarSymbol> { new GrammarSymbol("*", 0) };
	unique_ptr<GrammarSymbol> openingBrace = unique_ptr<GrammarSymbol> { new GrammarSymbol("(", 0) };
	unique_ptr<GrammarSymbol> closingBrace = unique_ptr<GrammarSymbol> { new GrammarSymbol(")", 0) };

	expression->addProduction( { term.get(), addOper.get(), expression.get() });
	expression->addProduction( { term.get() });

	term->addProduction( { factor.get(), multiOper.get(), term.get() });
	term->addProduction( { factor.get() });

	factor->addProduction( { openingBrace.get(), expression.get(), closingBrace.get() });
	factor->addProduction( { operand.get() });

	operand->addProduction( { constant.get() });
	operand->addProduction( { identifier.get() });

	std::vector<const GrammarSymbol*> nonterminals { expression.get(), term.get(), factor.get(), operand.get() };
	FirstTable first { nonterminals };

	auto expressionFirst = first(expression.get());
	ASSERT_THAT(expressionFirst, SizeIs(3));
	ASSERT_THAT(expressionFirst, ElementsAre(openingBrace.get(), constant.get(), identifier.get()));

	auto termFirst = first(term.get());
	ASSERT_THAT(termFirst, SizeIs(3));
	ASSERT_THAT(termFirst, ElementsAre(openingBrace.get(), constant.get(), identifier.get()));

	auto factorFirst = first(factor.get());
	ASSERT_THAT(factorFirst, SizeIs(3));
	ASSERT_THAT(factorFirst, ElementsAre(openingBrace.get(), constant.get(), identifier.get()));

	auto operandFirst = first(operand.get());
	ASSERT_THAT(operandFirst, SizeIs(2));
	ASSERT_THAT(operandFirst, ElementsAre(constant.get(), identifier.get()));

	auto constantFirst = first(constant.get());
	ASSERT_THAT(constantFirst, SizeIs(1));
	ASSERT_THAT(constantFirst, ElementsAre(constant.get()));

	auto identifierFirst = first(identifier.get());
	ASSERT_THAT(identifierFirst, SizeIs(1));
	ASSERT_THAT(identifierFirst, ElementsAre(identifier.get()));
}
