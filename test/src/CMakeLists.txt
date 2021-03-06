enable_testing()

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/)

add_library(testUtil util/ResourceHelpers.cpp util/ResourceHelpers.h)
target_include_directories(testUtil PUBLIC util)

add_executable(astTest gtest.cpp
        astTest/DeclarationSpecifiersTest.cpp
        astTest/DeclarationTest.cpp
        )
target_link_libraries(astTest gmock ast translation_unit)
add_test(astTest astTest)

add_executable(parserGeneratorTest gtest.cpp
        parserGeneratorTest/LR1ParserGeneratedTableTests.cpp
        )
target_link_libraries(parserGeneratorTest gmock parser driver testUtil)
add_test(parserGeneratorTest parserGeneratorTest)

add_executable(parserTest gtest.cpp
        parserTest/AcceptActionTest.cpp
        parserTest/BNFGrammarReaderTest.cpp
        parserTest/CanonicalCollectionTest.cpp
        parserTest/ClosureTest.cpp
        parserTest/FilePersistedParsingTableTest.cpp
        parserTest/FirstTableTest.cpp
        parserTest/GrammarBuilderTest.cpp
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
target_link_libraries(parserTest gmock parser driver testUtil)
add_test(parserTest parserTest)


add_executable(scannerTest gtest.cpp
        scannerTest/ScannerTest.cpp
        scannerTest/FiniteAutomatonTest.cpp
        scannerTest/LexFileScannerReaderTest.cpp
        scannerTest/StateTest.cpp
        scannerTest/TokenMatcher.h
        )
target_link_libraries(scannerTest gmock scanner testUtil)
add_test(scannerTest scannerTest)


add_executable(semantic_analyzerTest gtest.cpp
        semantic_analyzerTest/SemanticXmlOutputVisitorTest.cpp
        )
target_link_libraries(semantic_analyzerTest gmock semantic_analyzer)
add_test(semantic_analyzerTest semantic_analyzerTest)


add_executable(typesTest gtest.cpp
    typesTest/PrimitiveTest.cpp
    typesTest/TypeTest.cpp
    typesTest/FunctionTest.cpp
    )
target_link_libraries(typesTest gmock types)
add_test(typesTest typesTest)


add_executable(translation_unitTest gtest.cpp
        translation_unitTest/TranslationUnitTest.cpp
        )
target_link_libraries(translation_unitTest gmock translation_unit testUtil)
add_test(translation_unitTest translation_unitTest)


add_executable(codegenTest gtest.cpp
        codegenTest/ATandTInstructionSetTests.cpp
        codegenTest/StackMachineTests.cpp
        codegenTest/BasicBlockTests.cpp
        )
target_link_libraries(codegenTest gmock codegen)
add_test(codegenTest codegenTest)


add_executable(driverTest gtest.cpp
        driverTest/ConfigurationParserTest.cpp
        )
target_link_libraries(driverTest gmock driver translation_unit scanner util parser testUtil)
add_test(driverTest driverTest)


add_executable(functionalTest gtest.cpp
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
        functionalTest/ShiftOperatorsTest.cpp
        functionalTest/AssignmentOperatorsTest.cpp
        functionalTest/SemanticErrorsTest.cpp
        )
target_link_libraries(functionalTest gmock driver translation_unit scanner util parser testUtil)
add_test(functionalTest functionalTest)

