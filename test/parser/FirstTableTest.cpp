#include "parser/FirstTable.h"
#include "parser/BNFReader.h"
#include "parser/GrammarSymbol.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace testing;

TEST(FirstTable, computesFirstTableForGrammarRules) {
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	FirstTable firstTable { bnfReader.getRules() };

	auto nonterminals = bnfReader.getNonterminals();

	auto first0 = firstTable.firstSetForNonterminal(nonterminals.at(0));
	ASSERT_THAT(first0, SizeIs(4));
	ASSERT_THAT(first0.at(0)->getName(), Eq("'int'"));
	ASSERT_THAT(first0.at(1)->getName(), Eq("'char'"));
	ASSERT_THAT(first0.at(2)->getName(), Eq("'void'"));
	ASSERT_THAT(first0.at(3)->getName(), Eq("'float'"));

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
	BNFReader bnfReader { "resources/configuration/grammar.bnf" };

	FirstTable firstTable { bnfReader.getRules() };

	auto nonterminals = bnfReader.getNonterminals();

	auto first0 = firstTable.firstSetForNonterminal(nonterminals.at(0));
	ASSERT_THAT(first0, SizeIs(4));
	ASSERT_THAT(first0.at(0)->getName(), Eq("'int'"));
	ASSERT_THAT(first0.at(1)->getName(), Eq("'char'"));
	ASSERT_THAT(first0.at(2)->getName(), Eq("'void'"));
	ASSERT_THAT(first0.at(3)->getName(), Eq("'float'"));
}
