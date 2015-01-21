AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CPPFLAGS := -Isrc
CXXFLAGS = -std=c++14 -Wall -Wextra -O0 -pg -fprofile-arcs -ftest-coverage
#CXXFLAGS = -std=c++14 -Wall -Wextra -O3
AM_LDFLAGS = -lgcov
AM_DEFAULT_SOURCE_EXT = .cpp

bin_PROGRAMS = trans
trans_MAIN = src/trans.cpp
                
driver_MODULES = src/driver/Configuration.h src/driver/ConfigurationParser.h src/driver/ConfigurationParser.cpp \
				 src/driver/CompilerComponentsFactory.h src/driver/CompilerComponentsFactory.cpp \
				 src/driver/Driver.h src/driver/Driver.cpp \
				 src/driver/Compiler.h src/driver/Compiler.cpp \
				 src/driver/TranslationUnit.h src/driver/TranslationUnit.cpp
				 
translation_unit_MODULES = src/translation_unit/Context.h src/translation_unit/Context.cpp
				 
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
				 src/parser/TerminalNode.h src/parser/TerminalNode.cpp \
				 src/parser/SyntaxTreeBuilder.h \
				 src/parser/ParseTreeBuilder.h src/parser/ParseTreeBuilder.cpp \
				 src/parser/ErrorSyntaxTreeBuilder.h src/parser/ErrorSyntaxTreeBuilder.cpp \
				 src/parser/SyntaxTree.h \
				 src/parser/ParseTree.h src/parser/ParseTree.cpp \
				 src/parser/ParseTreeNodeVisitor.h \
				 src/parser/XmlOutputVisitor.h src/parser/XmlOutputVisitor.cpp \
				 src/parser/ParseTreeToSourceConverter.h src/parser/ParseTreeToSourceConverter.cpp
				 
				 
				 
semantic_MODULES = src/semantic_analyzer/SemanticAnalyzer.h src/semantic_analyzer/SemanticAnalyzer.cpp \
				   src/semantic_analyzer/SemanticXmlOutputVisitor.h src/semantic_analyzer/SemanticXmlOutputVisitor.cpp \
				   src/semantic_analyzer/SemanticAnalysisVisitor.h src/semantic_analyzer/SemanticAnalysisVisitor.cpp \
				   src/semantic_analyzer/CodeGeneratingVisitor.h src/semantic_analyzer/CodeGeneratingVisitor.cpp \
				   src/semantic_analyzer/TypeConverter.h src/semantic_analyzer/TypeConverter.cpp
				     
ast_MODULES = src/ast/Operator.h src/ast/Operator.cpp \
			  src/semantic_analyzer/AbstractSyntaxTreeBuilder.h src/ast/AbstractSyntaxTreeBuilder.cpp \
			  src/ast/AbstractSyntaxTree.h src/ast/AbstractSyntaxTree.cpp \
			  src/ast/AbstractSyntaxTreeNode.h \
			  src/ast/AbstractSyntaxTreeBuilderContext.h src/ast/AbstractSyntaxTreeBuilderContext.cpp \
			  src/ast/TerminalSymbol.h src/ast/TerminalSymbol.cpp \
			  src/ast/ContextualSyntaxNodeBuilder.h src/ast/ContextualSyntaxNodeBuilder.cpp \
			  src/ast/AssignmentExpressionList.h src/ast/AssignmentExpressionList.cpp \
			  src/ast/Expression.h src/ast/Expression.cpp \
			  src/ast/ExpressionList.h src/ast/ExpressionList.cpp \
			  src/ast/ArithmeticExpression.h src/ast/ArithmeticExpression.cpp \
			  src/ast/AssignmentExpression.h src/ast/AssignmentExpression.cpp \
			  src/ast/BitwiseExpression.h src/ast/BitwiseExpression.cpp \
			  src/ast/LogicalExpression.h src/ast/LogicalExpression.cpp \
			  src/ast/LogicalAndExpression.h src/ast/LogicalAndExpression.cpp \
			  src/ast/LogicalOrExpression.h src/ast/LogicalOrExpression.cpp \
			  src/ast/ShiftExpression.h src/ast/ShiftExpression.cpp \
			  src/ast/FunctionDefinition.h src/ast/FunctionDefinition.cpp \
			  src/ast/ListCarrier.h src/ast/ListCarrier.cpp \
			  src/ast/TypeCast.h src/ast/TypeCast.cpp \
		   	  src/ast/PointerCast.h src/ast/PointerCast.cpp \
			  src/ast/PostfixExpression.h src/ast/PostfixExpression.cpp \
			  src/ast/ArrayAccess.h src/ast/ArrayAccess.cpp \
			  src/ast/FunctionCall.h src/ast/FunctionCall.cpp \
			  src/ast/VariableDeclaration.h src/ast/VariableDeclaration.cpp \
			  src/ast/VariableDefinition.h src/ast/VariableDefinition.cpp \
			  src/ast/Declarator.h \
			  src/ast/DirectDeclarator.h src/ast/DirectDeclarator.cpp \
			  src/ast/Identifier.h src/ast/Identifier.cpp \
			  src/ast/ArrayDeclarator.h src/ast/ArrayDeclarator.cpp \
			  src/ast/FunctionDeclarator.h src/ast/FunctionDeclarator.cpp \
			  src/ast/DeclarationList.h src/ast/DeclarationList.cpp \
			  src/ast/ComparisonExpression.h src/ast/ComparisonExpression.cpp \
			  src/ast/UnaryExpression.h src/ast/UnaryExpression.cpp \
			  src/ast/PrefixExpression.h src/ast/PrefixExpression.cpp \
			  src/ast/Term.h src/ast/Term.cpp \
			  src/ast/Pointer.h src/ast/Pointer.cpp \
			  src/ast/IOStatement.h src/ast/IOStatement.cpp \
			  src/ast/JumpStatement.h src/ast/JumpStatement.cpp \
			  src/ast/ReturnStatement.h src/ast/ReturnStatement.cpp \
			  src/ast/LoopHeader.h src/ast/LoopHeader.cpp \
			  src/ast/WhileLoopHeader.h src/ast/WhileLoopHeader.cpp \
			  src/ast/ForLoopHeader.h src/ast/ForLoopHeader.cpp \
			  src/ast/IfStatement.h src/ast/IfStatement.cpp \
			  src/ast/IfElseStatement.h src/ast/IfElseStatement.cpp \
			  src/ast/LoopStatement.h src/ast/LoopStatement.cpp \
			  src/ast/FormalArgument.h src/ast/FormalArgument.cpp \
			  src/ast/Block.h src/ast/Block.cpp \
			  src/ast/TranslationUnit.h src/ast/TranslationUnit.cpp \
			  src/ast/TypeSpecifier.h src/ast/TypeSpecifier.cpp \
			  src/ast/SingleOperandExpression.h src/ast/SingleOperandExpression.cpp \
			  src/ast/DoubleOperandExpression.h src/ast/DoubleOperandExpression.cpp \
  	  	      src/ast/AbstractSyntaxTreeVisitor.h \
			  src/ast/types/Type.h src/ast/types/Type.cpp \
			  src/ast/types/BaseType.h src/ast/types/BaseType.cpp \
			  src/ast/types/NumericType.h src/ast/types/NumericType.cpp \
			  src/ast/types/Void.h src/ast/types/Void.cpp \
			  src/ast/types/Function.h src/ast/types/Function.cpp \
			  src/ast/DereferencedDeclaration.h src/ast/DereferencedDeclaration.cpp
			 
				 			   
code_generator_MODULES = src/code_generator/symbol_table.h src/code_generator/symbol_table.cpp \
						 src/code_generator/ValueScope.h src/code_generator/ValueScope.cpp \
						 src/code_generator/quadruple.h src/code_generator/quadruple.cpp \
                		 src/code_generator/register.h src/code_generator/register.cpp \
                		 src/code_generator/code_generator.h src/code_generator/code_generator.cpp \
                		 src/code_generator/LabelEntry.h src/code_generator/LabelEntry.cpp \
                		 src/code_generator/ValueEntry.h src/code_generator/ValueEntry.cpp	\
                		 src/code_generator/FunctionEntry.h src/code_generator/FunctionEntry.cpp		   
				  
util_MODULES = src/util/Logger.h src/util/Logger.cpp src/util/LogManager.h src/util/LogManager.cpp

noinst_LTLIBRARIES = libtrans.la
libtrans_la_SOURCES = $(driver_MODULES) $(translation_unit_MODULES) $(scanner_MODULES) $(parser_MODULES) $(semantic_MODULES) $(ast_MODULES) $(code_generator_MODULES) $(util_MODULES)
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
					  test/parser/ProductionTest.cpp \
					  test/parser/ParseTreeNodeTest.cpp \
					  test/parser/TerminalNodeTest.cpp \
					  test/parser/XmlOutputVisitorTest.cpp \
					  test/parser/ParseTreeTest.cpp \
					  test/parser/ParseTreeToSourceConverterTest.cpp
					  
semantic_test_SOURCES = test/semantic_analyzer/SemanticXmlOutputVisitorTest.cpp \
						test/semantic_analyzer/SemanticAnalysisVisitorTest.cpp \
						test/semantic_analyzer/TypeConverterTest.cpp
						
ast_test_SOURCES = test/ast/types/BaseTypeTest.cpp test/ast/types/FunctionTest.cpp test/ast/types/TypeTest.cpp

gtest_SOURCES = test/gtest.cpp $(driver_test_SOURCES) $(scanner_test_SOURCES) $(parser_test_SOURCES) $(semantic_test_SOURCES) $(ast_test_SOURCES)
gtest_LDADD = test/lib/gtest/libgtest.a test/lib/gmock/libgmock.a libtrans.la -lpthread
gtest_CXXFLAGS = -Itest/lib/gtest/include -Itest/lib/gmock/include 

MOSTLYCLEANFILES = *.gcda *.gcno *.gcov
