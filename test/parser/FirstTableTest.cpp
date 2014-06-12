#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFReader.h"
#include "parser/GrammarSymbol.h"
#include "parser/Grammar.h"
#include "parser/FirstTable.h"

using namespace testing;
using std::shared_ptr;
using std::vector;

TEST(FirstTable, computesFirstTableForGrammarRules) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };
	Grammar grammar = bnfReader.getGrammar();

	FirstTable first { grammar.nonterminals };

	auto first0 = first(grammar.nonterminals.at(0));
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

	shared_ptr<GrammarSymbol> expression = std::make_shared<GrammarSymbol>("<expr>", 0);
	shared_ptr<GrammarSymbol> term = std::make_shared<GrammarSymbol>("<term>", 0);
	shared_ptr<GrammarSymbol> factor = std::make_shared<GrammarSymbol>("<factor>", 0);
	shared_ptr<GrammarSymbol> operand = std::make_shared<GrammarSymbol>("<operand>", 0);
	shared_ptr<GrammarSymbol> identifier = std::make_shared<GrammarSymbol>("identifier", 0);
	shared_ptr<GrammarSymbol> constant = std::make_shared<GrammarSymbol>("constant", 0);
	shared_ptr<GrammarSymbol> addOper = std::make_shared<GrammarSymbol>("+", 0);
	shared_ptr<GrammarSymbol> multiOper = std::make_shared<GrammarSymbol>("*", 0);
	shared_ptr<GrammarSymbol> openingBrace = std::make_shared<GrammarSymbol>("(", 0);
	shared_ptr<GrammarSymbol> closingBrace = std::make_shared<GrammarSymbol>(")", 0);

	expression->addProduction( { term, addOper, expression });
	expression->addProduction( { term });

	term->addProduction( { factor, multiOper, term });
	term->addProduction( { factor });

	factor->addProduction( { openingBrace, expression, closingBrace });
	factor->addProduction( { operand });

	operand->addProduction( { constant });
	operand->addProduction( { identifier });

	FirstTable first { { expression, term, factor, operand } };

	auto expressionFirst = first(expression);
	ASSERT_THAT(expressionFirst, SizeIs(3));
	ASSERT_THAT(expressionFirst, ElementsAre(openingBrace, constant, identifier));

	auto termFirst = first(term);
	ASSERT_THAT(termFirst, SizeIs(3));
	ASSERT_THAT(termFirst, ElementsAre(openingBrace, constant, identifier));

	auto factorFirst = first(factor);
	ASSERT_THAT(factorFirst, SizeIs(3));
	ASSERT_THAT(factorFirst, ElementsAre(openingBrace, constant, identifier));

	auto operandFirst = first(operand);
	ASSERT_THAT(operandFirst, SizeIs(2));
	ASSERT_THAT(operandFirst, ElementsAre(constant, identifier));

	auto constantFirst = first(constant);
	ASSERT_THAT(constantFirst, SizeIs(1));
	ASSERT_THAT(constantFirst, ElementsAre(constant));

	auto identifierFirst = first(identifier);
	ASSERT_THAT(identifierFirst, SizeIs(1));
	ASSERT_THAT(identifierFirst, ElementsAre(identifier));
}
