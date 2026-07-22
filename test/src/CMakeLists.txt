add_library(testUtil util/ResourceHelpers.cpp util/ResourceHelpers.h)
target_include_directories(testUtil PUBLIC util)

add_executable(astTest
        astTest/DeclarationSpecifiersTest.cpp
        astTest/DeclarationTest.cpp
        )
target_link_libraries(astTest PRIVATE GTest::gmock_main ast translation_unit)
add_test(NAME astTest COMMAND astTest)

add_executable(parserGeneratorTest
        parserGeneratorTest/LR1ParserGeneratedTableTests.cpp
        )
target_link_libraries(parserGeneratorTest PRIVATE GTest::gmock_main parser driver testUtil)
add_test(NAME parserGeneratorTest COMMAND parserGeneratorTest)

add_executable(parserTest
        parserTest/AcceptActionTest.cpp
        parserTest/ActionValueTest.cpp
        parserTest/BNFGrammarReaderTest.cpp
        parserTest/CanonicalCollectionTest.cpp
        parserTest/ClosureTest.cpp
        parserTest/FilePersistedParsingTableTest.cpp
        parserTest/FirstTableTest.cpp
        parserTest/GrammarBuilderTest.cpp
        parserTest/GrammarTerminalMapTest.cpp
        parserTest/LookaheadActionTableTest.cpp
        parserTest/LR1ItemTest.cpp
        parserTest/LR1ParserTest.cpp
        parserTest/ParseTreeNodeTest.cpp
        parserTest/ParseTreeTest.cpp
        parserTest/ParseTreeToSourceConverterTest.cpp
        parserTest/ProductionTest.cpp
        parserTest/ShiftActionTest.cpp
        parserTest/TerminalNodeTest.cpp
        parserTest/TokenMatcher.h
        parserTest/TokenStreamTest.cpp
        parserTest/XmlOutputVisitorTest.cpp
        )
target_link_libraries(parserTest PRIVATE GTest::gmock_main parser driver testUtil)
add_test(NAME parserTest COMMAND parserTest)


add_executable(scannerTest
        scannerTest/ScannerTest.cpp
        scannerTest/FiniteAutomatonTest.cpp
        scannerTest/LexFileScannerReaderTest.cpp
        scannerTest/StateTest.cpp
        scannerTest/TokenMatcher.h
        )
target_link_libraries(scannerTest PRIVATE GTest::gmock_main scanner testUtil)
add_test(NAME scannerTest COMMAND scannerTest)


add_executable(semantic_analyzerTest
        semantic_analyzerTest/SemanticXmlOutputVisitorTest.cpp
        semantic_analyzerTest/SymbolTableTest.cpp
        )
target_link_libraries(semantic_analyzerTest PRIVATE GTest::gmock_main semantic_analyzer)
add_test(NAME semantic_analyzerTest COMMAND semantic_analyzerTest)


add_executable(typesTest
    typesTest/PrimitiveTest.cpp
    typesTest/TypeTest.cpp
    typesTest/FunctionTest.cpp
    )
target_link_libraries(typesTest PRIVATE GTest::gmock_main types)
add_test(NAME typesTest COMMAND typesTest)


add_executable(translation_unitTest
        translation_unitTest/TranslationUnitTest.cpp
        )
target_link_libraries(translation_unitTest PRIVATE GTest::gmock_main translation_unit testUtil)
add_test(NAME translation_unitTest COMMAND translation_unitTest)


add_executable(codegenTest
        codegenTest/ATandTInstructionSetTests.cpp
        codegenTest/StackMachineTests.cpp
        codegenTest/BasicBlockTests.cpp
        )
target_link_libraries(codegenTest PRIVATE GTest::gmock_main codegen)
add_test(NAME codegenTest COMMAND codegenTest)


add_executable(driverTest
        driverTest/ConfigurationParserTest.cpp
        )
target_link_libraries(driverTest PRIVATE GTest::gmock_main driver translation_unit scanner util parser testUtil)
add_test(NAME driverTest COMMAND driverTest)


add_executable(functionalTest
        functionalTest/TestFixtures.h
        functionalTest/TestFixtures.cpp
        functionalTest/CompilerInitTest.cpp
        functionalTest/ConditionalTest.cpp
        functionalTest/BasicMathTest.cpp
        functionalTest/IncrementsDecrementsTest.cpp
        functionalTest/PointersTest.cpp
        functionalTest/VoidTest.cpp
        functionalTest/InputOutputTest.cpp
        functionalTest/ForLoopTest.cpp
        functionalTest/WhileLoopTest.cpp
        functionalTest/FactorialTest.cpp
        functionalTest/FibonacciTest.cpp
        functionalTest/FunctionCallTest.cpp
        functionalTest/LogicalOperatorsTest.cpp
        functionalTest/UnaryOperatorsTest.cpp
        functionalTest/ShiftOperatorsTest.cpp
        functionalTest/AssignmentOperatorsTest.cpp
        functionalTest/SemanticErrorsTest.cpp
        functionalTest/ControlFlowTest.cpp
        functionalTest/ExpressionTest.cpp
        functionalTest/ScopeTest.cpp
        functionalTest/IntegrationTest.cpp
        functionalTest/GlobalVariablesTest.cpp
        functionalTest/FuzzCrashRegressionTest.cpp
        )
target_link_libraries(functionalTest PRIVATE GTest::gmock_main driver translation_unit scanner util parser testUtil)
add_test(NAME functionalTest COMMAND functionalTest)
