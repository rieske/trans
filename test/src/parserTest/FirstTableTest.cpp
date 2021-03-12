#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/GrammarSymbol.h"
#include "parser/Grammar.h"
#include "parser/FirstTable.h"

#include "ResourceHelpers.h"

using namespace testing;
using namespace parser;

TEST(FirstTable, computesFirstTableForGrammarRules) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("grammars/grammar_original.bnf"));

    FirstTable first { grammar };

    auto first0 = first(grammar.symbolId("<program>"));
    EXPECT_THAT(first0, SizeIs(4));
    EXPECT_THAT(first0, UnorderedElementsAre(
                grammar.symbolId("int"), grammar.symbolId("char"),
                grammar.symbolId("void"), grammar.symbolId("float"))
    );

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
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getTestResourcePath("grammars/expression_grammar.bnf"));

    FirstTable first { grammar };

    auto expressionFirst = first(grammar.symbolId("<expr>"));
    EXPECT_THAT(expressionFirst, SizeIs(3));
    EXPECT_THAT(expressionFirst, UnorderedElementsAre(grammar.symbolId("("), grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto termFirst = first(grammar.symbolId("<term>"));
    EXPECT_THAT(termFirst, SizeIs(3));
    EXPECT_THAT(termFirst, UnorderedElementsAre(grammar.symbolId("("), grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto factorFirst = first(grammar.symbolId("<factor>"));
    EXPECT_THAT(factorFirst, SizeIs(3));
    EXPECT_THAT(factorFirst, UnorderedElementsAre(grammar.symbolId("("), grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto operandFirst = first(grammar.symbolId("<operand>"));
    EXPECT_THAT(operandFirst, SizeIs(2));
    EXPECT_THAT(operandFirst, UnorderedElementsAre(grammar.symbolId("constant"), grammar.symbolId("identifier")));

    auto constantFirst = first(grammar.symbolId("constant"));
    EXPECT_THAT(constantFirst, SizeIs(1));
    EXPECT_THAT(constantFirst, ElementsAre(grammar.symbolId("constant")));

    auto identifierFirst = first(grammar.symbolId("identifier"));
    EXPECT_THAT(identifierFirst, SizeIs(1));
    EXPECT_THAT(identifierFirst, ElementsAre(grammar.symbolId("identifier")));
}

