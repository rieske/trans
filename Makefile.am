AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

if COND_DEBUG
COMPILE_FLAGS = -g -O0 --coverage --no-inline
AM_LDFLAGS = -lgcov
else
COMPILE_FLAGS = -O3 -DNDEBUG
endif

AM_CXXFLAGS = -std=c++14 -Wall -Wextra -p -Isrc $(COMPILE_FLAGS)
AM_DEFAULT_SOURCE_EXT = .cpp


driver_SOURCES = src/driver/Configuration.h src/driver/ConfigurationParser.h src/driver/ConfigurationParser.cpp \
				 src/driver/CompilerComponentsFactory.h src/driver/CompilerComponentsFactory.cpp \
				 src/driver/Driver.h src/driver/Driver.cpp \
				 src/driver/Compiler.h src/driver/Compiler.cpp \
				 src/driver/TranslationUnit.h src/driver/TranslationUnit.cpp

translation_unit_SOURCES = src/translation_unit/Context.h src/translation_unit/Context.cpp

scanner_SOURCES = src/scanner/Scanner.h src/scanner/FiniteAutomatonScanner.h src/scanner/FiniteAutomatonScanner.cpp \
				  src/scanner/State.h src/scanner/State.cpp \
				  src/scanner/IdentifierState.h src/scanner/IdentifierState.cpp \
				  src/scanner/StringLiteralState.h src/scanner/StringLiteralState.cpp \
				  src/scanner/EOLCommentState.h src/scanner/EOLCommentState.cpp \
				  src/scanner/Token.h src/scanner/Token.cpp \
				  src/scanner/LexFileFiniteAutomaton.h src/scanner/LexFileFiniteAutomaton.cpp \
				  src/scanner/FiniteAutomaton.h src/scanner/FiniteAutomaton.cpp

parser_SOURCES = src/parser/Parser.h src/parser/LR1Parser.h src/parser/LR1Parser.cpp \
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
				 src/parser/LR1Strategy.h src/parser/LR1Strategy.cpp \
				 src/parser/LALR1Strategy.h src/parser/LALR1Strategy.cpp \
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
				 src/parser/ParseTreeToSourceConverter.h src/parser/ParseTreeToSourceConverter.cpp \
				 src/parser/SyntaxTreeVisitor.h

semantic_SOURCES = src/semantic_analyzer/SemanticAnalyzer.h src/semantic_analyzer/SemanticAnalyzer.cpp \
				   src/semantic_analyzer/SemanticXmlOutputVisitor.h src/semantic_analyzer/SemanticXmlOutputVisitor.cpp \
				   src/semantic_analyzer/SemanticAnalysisVisitor.h src/semantic_analyzer/SemanticAnalysisVisitor.cpp \
				   src/semantic_analyzer/TypeConverter.h src/semantic_analyzer/TypeConverter.cpp \
				   src/semantic_analyzer/LabelEntry.h src/semantic_analyzer/LabelEntry.cpp \
        		   src/semantic_analyzer/ValueEntry.h src/semantic_analyzer/ValueEntry.cpp	\
        		   src/semantic_analyzer/FunctionEntry.h src/semantic_analyzer/FunctionEntry.cpp \
        		   src/semantic_analyzer/SymbolTable.h src/semantic_analyzer/SymbolTable.cpp \
				   src/semantic_analyzer/ValueScope.h src/semantic_analyzer/ValueScope.cpp

ast_SOURCES = src/ast/Operator.h src/ast/Operator.cpp \
			  src/semantic_analyzer/AbstractSyntaxTreeBuilder.h src/ast/AbstractSyntaxTreeBuilder.cpp \
			  src/ast/AbstractSyntaxTree.h src/ast/AbstractSyntaxTree.cpp \
			  src/ast/AbstractSyntaxTreeNode.h \
			  src/ast/AbstractSyntaxTreeBuilderContext.h src/ast/AbstractSyntaxTreeBuilderContext.cpp \
			  src/ast/TerminalSymbol.h src/ast/TerminalSymbol.cpp \
			  src/ast/ContextualSyntaxNodeBuilder.h src/ast/ContextualSyntaxNodeBuilder.cpp \
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
			  src/ast/TypeCast.h src/ast/TypeCast.cpp \
			  src/ast/PostfixExpression.h src/ast/PostfixExpression.cpp \
			  src/ast/ArrayAccess.h src/ast/ArrayAccess.cpp \
			  src/ast/FunctionCall.h src/ast/FunctionCall.cpp \
			  src/ast/Declarator.h src/ast/Declarator.cpp \
			  src/ast/DirectDeclarator.h src/ast/DirectDeclarator.cpp \
			  src/ast/Identifier.h src/ast/Identifier.cpp \
			  src/ast/ArrayDeclarator.h src/ast/ArrayDeclarator.cpp \
			  src/ast/FunctionDeclarator.h src/ast/FunctionDeclarator.cpp \
			  src/ast/ComparisonExpression.h src/ast/ComparisonExpression.cpp \
			  src/ast/UnaryExpression.h src/ast/UnaryExpression.cpp \
			  src/ast/PrefixExpression.h src/ast/PrefixExpression.cpp \
			  src/ast/IdentifierExpression.h src/ast/IdentifierExpression.cpp \
			  src/ast/ConstantExpression.h src/ast/ConstantExpression.cpp \
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
			  src/ast/SingleOperandExpression.h src/ast/SingleOperandExpression.cpp \
			  src/ast/DoubleOperandExpression.h src/ast/DoubleOperandExpression.cpp \
  	  	      src/ast/AbstractSyntaxTreeVisitor.h \
			  src/ast/types_deprecated/Type.h src/ast/types_deprecated/Type.cpp \
			  src/ast/types_deprecated/BaseType.h src/ast/types_deprecated/BaseType.cpp \
			  src/ast/types_deprecated/NumericType.h src/ast/types_deprecated/NumericType.cpp \
			  src/ast/types_deprecated/Void.h src/ast/types_deprecated/Void.cpp \
			  src/ast/types_deprecated/Function.h src/ast/types_deprecated/Function.cpp \
			  src/ast/TypeSpecifier.h src/ast/TypeSpecifier.cpp \
			  src/ast/TypeQualifier.h \
			  src/ast/StorageSpecifier.h src/ast/StorageSpecifier.cpp \
			  src/ast/DeclarationSpecifiers.h src/ast/DeclarationSpecifiers.cpp \
			  src/ast/Constant.h src/ast/Constant.cpp \
			  src/ast/InitializedDeclarator.h src/ast/InitializedDeclarator.cpp \
			  src/ast/Declaration.h src/ast/Declaration.cpp \
			  src/ast/TranslationUnitContextAware.h src/ast/TranslationUnitContextAware.cpp

type_SOURCES = src/ast/types/FundamentalType.h src/ast/types/FundamentalType.cpp \
			   src/ast/types/FunctionType.h src/ast/types/FunctionType.cpp \
			   src/ast/types/ArrayType.h src/ast/types/ArrayType.cpp \
			   src/ast/types/PointerType.h src/ast/types/PointerType.cpp \
			   src/ast/types/VoidType.h src/ast/types/VoidType.cpp \
			   src/ast/types/IntegralType.h src/ast/types/IntegralType.cpp \
			   src/ast/types/FloatingType.h src/ast/types/FloatingType.cpp

code_generator_SOURCES = src/codegen/Value.h src/codegen/Value.cpp \
				   		 src/codegen/CodeGeneratingVisitor.h src/codegen/CodeGeneratingVisitor.cpp \
				   		 src/codegen/QuadrupleGenerator.h src/codegen/QuadrupleGenerator.cpp \
                		 src/codegen/AssemblyGenerator.h src/codegen/AssemblyGenerator.cpp \
                		 src/codegen/StackMachine.h src/codegen/StackMachine.cpp \
                		 src/codegen/Register.h src/codegen/Register.cpp \
                		 src/codegen/InstructionSet.h \
                		 src/codegen/IntelInstructionSet.h src/codegen/IntelInstructionSet.cpp \
                		 src/codegen/ATandTInstructionSet.h src/codegen/ATandTInstructionSet.cpp \
                		 src/codegen/Amd64Registers.h src/codegen/Amd64Registers.cpp \
                		 src/codegen/quadruples/Quadruple.h \
                		 src/codegen/quadruples/SingleOperandQuadruple.h src/codegen/quadruples/SingleOperandQuadruple.cpp \
                		 src/codegen/quadruples/DoubleOperandQuadruple.h src/codegen/quadruples/DoubleOperandQuadruple.cpp \
                		 src/codegen/quadruples/Input.h src/codegen/quadruples/Input.cpp \
                		 src/codegen/quadruples/Output.h src/codegen/quadruples/Output.cpp \
                		 src/codegen/quadruples/StartProcedure.h src/codegen/quadruples/StartProcedure.cpp \
                		 src/codegen/quadruples/EndProcedure.h src/codegen/quadruples/EndProcedure.cpp \
                		 src/codegen/quadruples/Label.h src/codegen/quadruples/Label.cpp \
                		 src/codegen/quadruples/Jump.h src/codegen/quadruples/Jump.cpp \
                		 src/codegen/quadruples/ValueCompare.h src/codegen/quadruples/ValueCompare.cpp \
                		 src/codegen/quadruples/ZeroCompare.h src/codegen/quadruples/ZeroCompare.cpp \
                		 src/codegen/quadruples/AddressOf.h src/codegen/quadruples/AddressOf.cpp \
                		 src/codegen/quadruples/Dereference.h src/codegen/quadruples/Dereference.cpp \
                		 src/codegen/quadruples/UnaryMinus.h src/codegen/quadruples/UnaryMinus.cpp \
                		 src/codegen/quadruples/Assign.h src/codegen/quadruples/Assign.cpp \
                		 src/codegen/quadruples/AssignConstant.h src/codegen/quadruples/AssignConstant.cpp \
                		 src/codegen/quadruples/LvalueAssign.h src/codegen/quadruples/LvalueAssign.cpp \
                		 src/codegen/quadruples/Argument.h src/codegen/quadruples/Argument.cpp \
                		 src/codegen/quadruples/Call.h src/codegen/quadruples/Call.cpp \
                		 src/codegen/quadruples/Return.h src/codegen/quadruples/Return.cpp \
                		 src/codegen/quadruples/Retrieve.h src/codegen/quadruples/Retrieve.cpp \
                		 src/codegen/quadruples/Xor.h src/codegen/quadruples/Xor.cpp \
                		 src/codegen/quadruples/Or.h src/codegen/quadruples/Or.cpp \
                		 src/codegen/quadruples/And.h src/codegen/quadruples/And.cpp \
                		 src/codegen/quadruples/Add.h src/codegen/quadruples/Add.cpp \
                		 src/codegen/quadruples/Sub.h src/codegen/quadruples/Sub.cpp \
                		 src/codegen/quadruples/Mul.h src/codegen/quadruples/Mul.cpp \
                		 src/codegen/quadruples/Div.h src/codegen/quadruples/Div.cpp \
                		 src/codegen/quadruples/Mod.h src/codegen/quadruples/Mod.cpp \
                		 src/codegen/quadruples/Inc.h src/codegen/quadruples/Inc.cpp \
                		 src/codegen/quadruples/Dec.h src/codegen/quadruples/Dec.cpp

util_SOURCES = src/util/Logger.h src/util/Logger.cpp src/util/LogManager.h src/util/LogManager.cpp

trans_MAIN = src/trans.cpp

trans_CORE = $(driver_SOURCES) $(translation_unit_SOURCES) $(scanner_SOURCES) $(parser_SOURCES) \
			 $(type_SOURCES) $(semantic_SOURCES) $(ast_SOURCES) $(code_generator_SOURCES) $(util_SOURCES)
noinst_LIBRARIES = libtrans.a
libtrans_a_SOURCES = $(trans_CORE)
trans_SOURCES = $(trans_MAIN)
trans_LDADD = libtrans.a
EXTRA_DIST = autogen.sh

bin_PROGRAMS = trans
noinst_PROGRAMS = gtest quicktest
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
					  test/parser/FilePersistedParsingTableTest.cpp \
					  test/parser/GeneratedParsingTableTest.cpp \
					  test/parser/AcceptActionTest.cpp \
					  test/parser/ShiftActionTest.cpp \
					  test/parser/TokenStreamTest.cpp \
					  test/parser/ProductionTest.cpp \
					  test/parser/ParseTreeNodeTest.cpp \
					  test/parser/TerminalNodeTest.cpp \
					  test/parser/XmlOutputVisitorTest.cpp \
					  test/parser/ParseTreeTest.cpp \
					  test/parser/ParseTreeToSourceConverterTest.cpp
					  
parser_it_SOURCES = test/parser/LR1ParserGeneratedTableTests.cpp

semantic_test_SOURCES = test/semantic_analyzer/SemanticXmlOutputVisitorTest.cpp \
						test/semantic_analyzer/TypeConverterTest.cpp

ast_test_SOURCES = test/ast/types/BaseTypeTest.cpp test/ast/types/FunctionTest.cpp test/ast/types/TypeTest.cpp \
				   test/ast/DeclarationSpecifiersTest.cpp test/ast/DeclarationTest.cpp
				   
codegen_test_SOURCES = test/codegen/StackMachineTests.cpp \
					   test/codegen/ATandTInstructionSetTests.cpp
					   
quicktest_SOURCES = test/gtest.cpp $(driver_test_SOURCES) $(scanner_test_SOURCES) $(parser_test_SOURCES) $(semantic_test_SOURCES) $(ast_test_SOURCES) \
				$(codegen_test_SOURCES)
quicktest_LDADD = $(gtest_LDADD)
quicktest_CXXFLAGS = $(gtest_CXXFLAGS)

gtest_SOURCES = $(quicktest_SOURCES) $(parser_it_SOURCES)
gtest_LDADD = test/lib/gtest/libgtest.a test/lib/gmock/libgmock.a libtrans.a -lpthread -lgcov
gtest_CXXFLAGS = -Itest/lib/gtest/include -Itest/lib/gmock/include -Isrc -std=c++14 -Wall -Wextra -p -g3 -O0

MOSTLYCLEANFILES = *.gcda *.gcno *.gcov *.lo \
				   src/*.gcno src/*.gcda \
				   src/util/*.gcno src/util/*.gcda src/util/*.lo \
				   src/driver/*.gcno src/driver/*.gcda src/driver/*.lo \
				   src/translation_unit/*.gcno src/translation_unit/*.gcda src/translation_unit/*.lo \
				   src/scanner/*.gcno src/scanner/*.gcda src/scanner/*.lo \
				   src/parser/*.gcno src/parser/*.gcda src/parser/*.lo \
				   src/ast/*.gcno src/ast/*.gcda src/ast/*.lo \
				   src/ast/types/*.gcno src/ast/types/*.gcda src/ast/types/*.lo \
				   src/semantic_analyzer/*.gcno src/semantic_analyzer/*.gcda src/semantic_analyzer/*.lo \
				   src/codegen/*.gcno src/codegen/*.gcda src/codegen/*.lo \
				   src/codegen/quadruples/*.gcno src/codegen/quadruples/*.gcda src/codegen/quadruples/*.lo
