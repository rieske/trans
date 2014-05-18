AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CPPFLAGS = -Isrc 
CXXFLAGS = -std=c++11 -Wall -Wextra -O0 -g -fprofile-arcs -ftest-coverage
AM_LDFLAGS = -lgcov
AM_DEFAULT_SOURCE_EXT = .cpp

bin_PROGRAMS = trans
trans_MAIN = src/trans.cpp
trans_MODULES =	src/parser/action.h src/parser/action.cpp \
				src/parser/node.h src/parser/node.cpp \
				src/parser/nonterminal_node.h src/parser/nonterminal_node.cpp \
				src/parser/terminal_node.h src/parser/terminal_node.cpp \
				src/semantic_analyzer/decl_node.h src/semantic_analyzer/decl_node.cpp \
				src/semantic_analyzer/decls_node.h src/semantic_analyzer/decls_node.cpp \
				src/semantic_analyzer/log_and_expr_node.h src/semantic_analyzer/log_and_expr_node.cpp \
				src/semantic_analyzer/log_expr_node.h src/semantic_analyzer/log_expr_node.cpp \
				src/semantic_analyzer/dir_decl_node.h src/semantic_analyzer/dir_decl_node.cpp \
				src/semantic_analyzer/add_expr_node.h src/semantic_analyzer/add_expr_node.cpp \
				src/semantic_analyzer/postfix_expr_node.h src/semantic_analyzer/postfix_expr_node.cpp \
				src/semantic_analyzer/a_expressions_node.h src/semantic_analyzer/a_expressions_node.cpp \
				src/semantic_analyzer/loop_hdr_node.h src/semantic_analyzer/loop_hdr_node.cpp \
				src/semantic_analyzer/ptr_node.h src/semantic_analyzer/ptr_node.cpp \
				src/semantic_analyzer/a_expr_node.h src/semantic_analyzer/a_expr_node.cpp \
				src/semantic_analyzer/eq_expr_node.h src/semantic_analyzer/eq_expr_node.cpp \
				src/semantic_analyzer/matched_node.h src/semantic_analyzer/matched_node.cpp \
				src/semantic_analyzer/term_node.h src/semantic_analyzer/term_node.cpp \
				src/semantic_analyzer/and_expr_node.h src/semantic_analyzer/and_expr_node.cpp \
				src/semantic_analyzer/expr_node.h src/semantic_analyzer/expr_node.cpp \
				src/semantic_analyzer/ml_expr_node.h src/semantic_analyzer/ml_expr_node.cpp \
				src/semantic_analyzer/factor_node.h src/semantic_analyzer/factor_node.cpp \
				src/semantic_analyzer/u_expr_node.h src/semantic_analyzer/u_expr_node.cpp \
				src/semantic_analyzer/block_node.h src/semantic_analyzer/block_node.cpp \
				src/semantic_analyzer/func_decl_node.h src/semantic_analyzer/func_decl_node.cpp \
				src/semantic_analyzer/unmatched_node.h src/semantic_analyzer/unmatched_node.cpp \
				src/semantic_analyzer/carrier_node.h src/semantic_analyzer/carrier_node.cpp \
				src/semantic_analyzer/or_expr_node.h src/semantic_analyzer/or_expr_node.cpp \
				src/semantic_analyzer/var_decl_node.h src/semantic_analyzer/var_decl_node.cpp \
				src/semantic_analyzer/cast_expr_node.h src/semantic_analyzer/cast_expr_node.cpp \
				src/semantic_analyzer/io_stmt_node.h src/semantic_analyzer/io_stmt_node.cpp \
				src/semantic_analyzer/param_decl_node.h src/semantic_analyzer/param_decl_node.cpp \
				src/semantic_analyzer/s_expr_node.h src/semantic_analyzer/s_expr_node.cpp \
				src/semantic_analyzer/xor_expr_node.h src/semantic_analyzer/xor_expr_node.cpp \
				src/semantic_analyzer/param_list_node.h src/semantic_analyzer/param_list_node.cpp \
				src/semantic_analyzer/jmp_stmt_node.h src/semantic_analyzer/jmp_stmt_node.cpp \
				src/code_generator/symbol_table.h src/code_generator/symbol_table.cpp \
				src/code_generator/symbol_entry.h src/code_generator/symbol_entry.cpp \
				src/code_generator/quadruple.h src/code_generator/quadruple.cpp \
                src/code_generator/register.h src/code_generator/register.cpp \
                src/code_generator/code_generator.h src/code_generator/code_generator.cpp
                
                
driver_MODULES = src/driver/Configuration.h src/driver/ConfigurationParser.h src/driver/ConfigurationParser.cpp \
				 src/driver/CompilerComponentsFactory.h src/driver/ConfigurableCompilerComponentsFactory.h src/driver/ConfigurableCompilerComponentsFactory.cpp \
				 src/driver/Driver.h src/driver/Driver.cpp \
				 src/driver/Compiler.h src/driver/TransCompiler.h src/driver/TransCompiler.cpp \
				 src/driver/TranslationUnit.h src/driver/SourceTranslationUnit.h src/driver/SourceTranslationUnit.cpp
				 
scanner_MODULES = src/scanner/Scanner.h src/scanner/FiniteAutomatonScanner.h src/scanner/FiniteAutomatonScanner.cpp \
				  src/scanner/State.h src/scanner/State.cpp \
				  src/scanner/IdentifierState.h src/scanner/IdentifierState.cpp \
				  src/scanner/StringLiteralState.h src/scanner/StringLiteralState.cpp \
				  src/scanner/EOLCommentState.h src/scanner/EOLCommentState.cpp \
				  src/scanner/Token.h src/scanner/Token.cpp \
				  src/scanner/StateMachineFactory.h src/scanner/FiniteAutomatonFactory.h src/scanner/FiniteAutomatonFactory.cpp \
				  src/scanner/StateMachine.h src/scanner/FiniteAutomaton.h src/scanner/FiniteAutomaton.cpp
				  
parser_MODULES = src/parser/Parser.h src/parser/LR1Parser.h src/parser/LR1Parser.cpp \
				 src/parser/ParsingTable.h src/parser/ParsingTable.cpp \
				 src/parser/BNFReader.h src/parser/BNFReader.cpp \
				 src/parser/Grammar.h src/parser/Grammar.cpp \
				 src/parser/GrammarSymbol.h src/parser/GrammarSymbol.cpp \
				 src/parser/TerminalSymbol.h src/parser/TerminalSymbol.cpp \
				 src/parser/NonterminalSymbol.h src/parser/NonterminalSymbol.cpp \
				 src/parser/GrammarRule.h src/parser/GrammarRule.cpp \
				 src/parser/GrammarRuleBuilder.h src/parser/GrammarRuleBuilder.cpp \
				 src/parser/FirstTable.h src/parser/FirstTable.cpp \
				 src/parser/LR1Item.h src/parser/LR1Item.cpp
				 
				 
semantic_MODULES = src/semantic_analyzer/SemanticComponentsFactory.h \
				   src/semantic_analyzer/ConfigurableSemanticComponentsFactory.h src/semantic_analyzer/ConfigurableSemanticComponentsFactory.cpp \
				   src/semantic_analyzer/SyntaxTreeBuilder.h src/semantic_analyzer/SyntaxTreeBuilder.cpp \
				   src/semantic_analyzer/SemanticSyntaxTreeBuilder.h src/semantic_analyzer/SemanticSyntaxTreeBuilder.cpp \
				   src/semantic_analyzer/ParseTreeBuilder.h src/semantic_analyzer/ParseTreeBuilder.cpp \
				   src/semantic_analyzer/SyntaxTree.h src/semantic_analyzer/SyntaxTree.cpp
				   
				  
util_MODULES = src/util/Logger.h src/util/Logger.cpp

noinst_LTLIBRARIES = libtrans.la
libtrans_la_SOURCES = $(trans_MODULES) $(driver_MODULES) $(scanner_MODULES) $(parser_MODULES) $(semantic_MODULES) $(util_MODULES)
trans_SOURCES = $(trans_MAIN)
trans_LDADD = libtrans.la

EXTRA_DIST = autogen.sh

noinst_PROGRAMS = gtest
check_PROGRAMS = gtest
TESTS = gtestAndReport.sh

driver_test_SOURCES = test/driver/ConfigurationParserTest.cpp test/driver/DriverTest.cpp test/driver/TransCompilerTest.cpp \
					  test/driver/SourceTranslationUnitTest.cpp
scanner_test_SOURCES = test/scanner/FiniteAutomatonScannerTest.cpp test/scanner/FiniteAutomatonTest.cpp test/scanner/FiniteAutomatonFactoryTest.cpp \
					   test/scanner/StateTest.cpp test/scanner/TokenMatcher.h
					   
parser_test_SOURCES = test/parser/GrammarTest.cpp test/parser/BNFReaderTest.cpp test/parser/FirstTableTest.cpp \
					  test/parser/GrammarRuleBuilderTest.cpp test/parser/LR1ItemTest.cpp

gtest_SOURCES = test/gtest.cpp $(driver_test_SOURCES) $(scanner_test_SOURCES) $(parser_test_SOURCES)
gtest_LDADD = test/lib/gtest/libgtest.a test/lib/gmock/libgmock.a libtrans.la -lpthread
gtest_CXXFLAGS = -Itest/lib/gtest/include -Itest/lib/gmock/include 

