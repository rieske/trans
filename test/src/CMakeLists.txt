add_library(testUtil util/ResourceHelpers.cpp util/ResourceHelpers.h)
target_include_directories(testUtil PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/util>
)
# Warnings/coverage for test helpers; include path stays test-local (not trans_src).
target_link_libraries(testUtil PUBLIC trans_common)
if(TRANS_ENABLE_COVERAGE)
  target_link_libraries(testUtil PUBLIC trans_coverage)
endif()

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
        codegenTest/UnimplementedArrayCodegenTest.cpp
        )
target_link_libraries(codegenTest PRIVATE GTest::gmock_main backend ast)
add_test(NAME codegenTest COMMAND codegenTest)


add_executable(driverTest
        driverTest/ConfigurationParserTest.cpp
        driverTest/DriverExitCodeTest.cpp
        )
target_link_libraries(driverTest PRIVATE GTest::gmock_main driver translation_unit scanner util parser testUtil)
add_test(NAME driverTest COMMAND driverTest)


add_executable(utilTest
        utilTest/ProcessTest.cpp
        )
target_link_libraries(utilTest PRIVATE GTest::gmock_main util)
add_test(NAME utilTest COMMAND utilTest)


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

# Split functionalTest across N gtest processes so ctest -j can run them in parallel.
# Adding new TEST() cases needs no CMake changes — gtest assigns each case to a shard by name.
# Override at configure time: cmake -DFUNCTIONAL_TEST_SHARDS=16 ...
# Note: make coverage runs ctest -j1 so shards do not race on gcov .gcda files.
set(FUNCTIONAL_TEST_SHARDS 8 CACHE STRING "Number of gtest shards for functionalTest (ctest -j runs them in parallel)")
if(NOT FUNCTIONAL_TEST_SHARDS MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "FUNCTIONAL_TEST_SHARDS must be a positive integer (got: ${FUNCTIONAL_TEST_SHARDS})")
endif()
math(EXPR functional_last_shard "${FUNCTIONAL_TEST_SHARDS} - 1")
foreach(shard RANGE 0 ${functional_last_shard})
  add_test(NAME functionalTest_shard${shard} COMMAND functionalTest)
  set_tests_properties(functionalTest_shard${shard} PROPERTIES
    LABELS "functional"
    ENVIRONMENT "GTEST_TOTAL_SHARDS=${FUNCTIONAL_TEST_SHARDS};GTEST_SHARD_INDEX=${shard}"
  )
endforeach()
