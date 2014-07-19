AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CPPFLAGS = -Isrc 
CXXFLAGS = -std=c++11 -Wall -Wextra -O0 -g -fprofile-arcs -ftest-coverage
#CXXFLAGS = -std=c++11 -Wall -Wextra -O3
AM_LDFLAGS = -lgcov
AM_DEFAULT_SOURCE_EXT = .cpp

bin_PROGRAMS = trans
trans_MAIN = src/trans.cpp
trans_MODULES =	src/code_generator/symbol_table.h src/code_generator/symbol_table.cpp \
				src/code_generator/symbol_entry.h src/code_generator/symbol_entry.cpp \
				src/code_generator/quadruple.h src/code_generator/quadruple.cpp \
                src/code_generator/register.h src/code_generator/register.cpp \
                src/code_generator/code_generator.h src/code_generator/code_generator.cpp
                
                
driver_MODULES = src/driver/Configuration.h src/driver/ConfigurationParser.h src/driver/ConfigurationParser.cpp \
				 src/driver/CompilerComponentsFactory.h src/driver/CompilerComponentsFactory.cpp \
				 src/driver/Driver.h src/driver/Driver.cpp \
				 src/driver/Compiler.h src/driver/Compiler.cpp \
				 src/driver/TranslationUnit.h src/driver/TranslationUnit.cpp
				 
scanner_MODULES = src/scanner/Scanner.h src/scanner/FiniteAutomatonScanner.h src/scanner/FiniteAutomatonScanner.cpp \
				  src/scanner/State.h src/scanner/State.cpp \
				  src/scanner/IdentifierState.h src/scanner/IdentifierState.cpp \
				  src/scanner/StringLiteralState.h src/scanner/StringLiteralState.cpp \
				  src/scanner/EOLCommentState.h src/scanner/EOLCommentState.cpp \
				  src/scanner/Token.h src/scanner/Token.cpp \
				  src/scanner/LexFileFiniteAutomaton.h src/scanner/LexFileFiniteAutomaton.cpp \
				  src/scanner/FiniteAutomaton.h src/scanner/FiniteAutomaton.cpp
				  
parser_MODULES = src/parser/Parser.h src/parser/LR1Parser.h src/parser/LR1Parser.cpp \
			     src/parser/LookaheadActionTable.h src/parser/LookaheadActionTable.cpp \
				 src/parser/ParsingTable.h src/parser/ParsingTable.cpp \
				 src/parser/ParsingTableReader.h src/parser/ParsingTableReader.cpp \
				 src/parser/FilePersistedParsingTable.h src/parser/FilePersistedParsingTable.cpp \
				 src/parser/GeneratedParsingTable.h src/parser/GeneratedParsingTable.cpp \
				 src/parser/BNFFileGrammar.h src/parser/BNFFileGrammar.cpp \
				 src/parser/Grammar.h src/parser/Grammar.cpp \
				 src/parser/GrammarSymbol.h src/parser/GrammarSymbol.cpp \
				 src/parser/Production.h src/parser/Production.cpp \
				 src/parser/FirstTable.h src/parser/FirstTable.cpp \
				 src/parser/Closure.h src/parser/Closure.cpp \
				 src/parser/GoTo.h src/parser/GoTo.cpp \
				 src/parser/CanonicalCollection.h src/parser/CanonicalCollection.cpp \
				 src/parser/LR1Item.h src/parser/LR1Item.cpp \
				 src/parser/TokenStream.h src/parser/TokenStream.cpp \
				 src/parser/Action.h src/parser/Action.cpp \
				 src/parser/ShiftAction.h src/parser/ShiftAction.cpp \
				 src/parser/ReduceAction.h src/parser/ReduceAction.cpp \
				 src/parser/ErrorAction.h src/parser/ErrorAction.cpp \
				 src/parser/AcceptAction.h src/parser/AcceptAction.cpp \
				 src/parser/ParseTreeNode.h src/parser/ParseTreeNode.cpp \
				 src/parser/SyntaxTreeBuilder.h \
				 src/parser/ParseTreeBuilder.h src/parser/ParseTreeBuilder.cpp \
				 src/parser/ErrorSyntaxTreeBuilder.h src/parser/ErrorSyntaxTreeBuilder.cpp \
				 src/parser/SyntaxTree.h src/parser/SyntaxTree.cpp \
				 src/parser/ParseTree.h src/parser/ParseTree.cpp \
				 src/parser/NonterminalNode.h src/parser/NonterminalNode.cpp \
				 src/parser/TerminalNode.h src/parser/TerminalNode.cpp \
				 src/parser/ParseTreeNodeVisitor.h src/parser/ParseTreeNodeVisitor.cpp \
				 src/parser/XmlOutputVisitor.h src/parser/XmlOutputVisitor.cpp
				 
				 
				 
semantic_MODULES = src/semantic_analyzer/SemanticAnalyzer.h src/semantic_analyzer/SemanticAnalyzer.cpp \
				   src/semantic_analyzer/SemanticTreeBuilder.h src/semantic_analyzer/SemanticTreeBuilder.cpp \
				   src/semantic_analyzer/AbstractSyntaxTree.h src/semantic_analyzer/AbstractSyntaxTree.cpp \
				   src/semantic_analyzer/AbstractSyntaxTreeNode.h src/semantic_analyzer/AbstractSyntaxTreeNode.cpp \
				   src/semantic_analyzer/AbstractSyntaxTreeBuilderContext.h src/semantic_analyzer/AbstractSyntaxTreeBuilderContext.cpp \
				   src/semantic_analyzer/NonterminalNode.h src/semantic_analyzer/NonterminalNode.cpp \
				   src/semantic_analyzer/TerminalSymbol.h src/semantic_analyzer/TerminalSymbol.cpp \
				   src/semantic_analyzer/AbstractSyntaxTreeVisitor.h src/semantic_analyzer/AbstractSyntaxTreeVisitor.cpp \
				   src/semantic_analyzer/NonterminalNodeFactory.h src/semantic_analyzer/NonterminalNodeFactory.cpp \
				   src/semantic_analyzer/AssignmentExpressionList.h src/semantic_analyzer/AssignmentExpressionList.cpp \
				   src/semantic_analyzer/Expression.h src/semantic_analyzer/Expression.cpp \
				   src/semantic_analyzer/AdditionExpression.h src/semantic_analyzer/AdditionExpression.cpp \
				   src/semantic_analyzer/AssignmentExpression.h src/semantic_analyzer/AssignmentExpression.cpp \
				   src/semantic_analyzer/BitwiseAndExpression.h src/semantic_analyzer/BitwiseAndExpression.cpp \
				   src/semantic_analyzer/BitwiseOrExpression.h src/semantic_analyzer/BitwiseOrExpression.cpp \
				   src/semantic_analyzer/BitwiseXorExpression.h src/semantic_analyzer/BitwiseXorExpression.cpp \
				   src/semantic_analyzer/LogicalAndExpression.h src/semantic_analyzer/LogicalAndExpression.cpp \
				   src/semantic_analyzer/LogicalOrExpression.h src/semantic_analyzer/LogicalOrExpression.cpp \
				   src/semantic_analyzer/ShiftExpression.h src/semantic_analyzer/ShiftExpression.cpp \
				   src/semantic_analyzer/FunctionDeclaration.h src/semantic_analyzer/FunctionDeclaration.cpp \
				   src/semantic_analyzer/Carrier.h src/semantic_analyzer/Carrier.cpp \
				   src/semantic_analyzer/CastExpression.h src/semantic_analyzer/CastExpression.cpp \
				   src/semantic_analyzer/PostfixExpression.h src/semantic_analyzer/PostfixExpression.cpp \
				   src/semantic_analyzer/VariableDeclaration.h src/semantic_analyzer/VariableDeclaration.cpp \
				   src/semantic_analyzer/Declaration.h src/semantic_analyzer/Declaration.cpp \
				   src/semantic_analyzer/DeclarationList.h src/semantic_analyzer/DeclarationList.cpp \
				   src/semantic_analyzer/DirectDeclaration.h src/semantic_analyzer/DirectDeclaration.cpp \
				   src/semantic_analyzer/EqualityExpression.h src/semantic_analyzer/EqualityExpression.cpp \
				   src/semantic_analyzer/ComparisonExpression.h src/semantic_analyzer/ComparisonExpression.cpp \
				   src/semantic_analyzer/UnaryExpression.h src/semantic_analyzer/UnaryExpression.cpp \
				   src/semantic_analyzer/Factor.h src/semantic_analyzer/Factor.cpp \
				   src/semantic_analyzer/Term.h src/semantic_analyzer/Term.cpp \
				   src/semantic_analyzer/Pointer.h src/semantic_analyzer/Pointer.cpp \
				   src/semantic_analyzer/IOStatement.h src/semantic_analyzer/IOStatement.cpp \
				   src/semantic_analyzer/JumpStatement.h src/semantic_analyzer/JumpStatement.cpp \
				   src/semantic_analyzer/LoopHeader.h src/semantic_analyzer/LoopHeader.cpp \
				   src/semantic_analyzer/MatchedNode.h src/semantic_analyzer/MatchedNode.cpp \
				   src/semantic_analyzer/UnmatchedNode.h src/semantic_analyzer/UnmatchedNode.cpp \
				   src/semantic_analyzer/ParameterList.h src/semantic_analyzer/ParameterList.cpp \
				   src/semantic_analyzer/ParameterDeclaration.h src/semantic_analyzer/ParameterDeclaration.cpp \
				   src/semantic_analyzer/Block.h src/semantic_analyzer/Block.cpp \
				   src/semantic_analyzer/SemanticXmlOutputVisitor.h src/semantic_analyzer/SemanticXmlOutputVisitor.cpp
				   
				  
util_MODULES = src/util/Logger.h src/util/Logger.cpp src/util/LogManager.h src/util/LogManager.cpp

noinst_LTLIBRARIES = libtrans.la
libtrans_la_SOURCES = $(trans_MODULES) $(driver_MODULES) $(scanner_MODULES) $(parser_MODULES) $(semantic_MODULES) $(util_MODULES)
trans_SOURCES = $(trans_MAIN)
trans_LDADD = libtrans.la

EXTRA_DIST = autogen.sh

noinst_PROGRAMS = gtest
check_PROGRAMS = gtest
TESTS = gtestAndReport.sh

driver_test_SOURCES = test/driver/ConfigurationParserTest.cpp test/driver/CompilerTest.cpp \
					  test/driver/TranslationUnitTest.cpp
scanner_test_SOURCES = test/scanner/FiniteAutomatonScannerTest.cpp test/scanner/FiniteAutomatonTest.cpp test/scanner/LexFileFiniteAutomatonTest.cpp \
					   test/scanner/StateTest.cpp test/scanner/TokenMatcher.h
					   
parser_test_SOURCES = test/parser/BNFFileGrammarTest.cpp test/parser/FirstTableTest.cpp \
					  test/parser/LR1ItemTest.cpp test/parser/ClosureTest.cpp \
					  test/parser/GoToTest.cpp test/parser/CanonicalCollectionTest.cpp \
					  test/parser/LR1ParserTest.cpp \
					  test/parser/FilePersistedParsingTableTest.cpp test/parser/GeneratedParsingTableTest.cpp \
					  test/parser/AcceptActionTest.cpp \
					  test/parser/ShiftActionTest.cpp \
					  test/parser/TokenStreamTest.cpp \
					  test/parser/ProductionTest.cpp

gtest_SOURCES = test/gtest.cpp $(driver_test_SOURCES) $(scanner_test_SOURCES) $(parser_test_SOURCES)
gtest_LDADD = test/lib/gtest/libgtest.a test/lib/gmock/libgmock.a libtrans.la -lpthread
gtest_CXXFLAGS = -Itest/lib/gtest/include -Itest/lib/gmock/include 

