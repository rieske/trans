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
    astTest/CSNBCreatorsStubTest.cpp
    astTest/ParseEnvironmentTest.cpp
    astTest/ExpressionDualTypeTest.cpp
        astTest/TypeSpecifierTest.cpp
        )
target_link_libraries(astTest PRIVATE semantic_analyzer GTest::gmock_main ast translation_unit)
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
        scannerTest/ScannerTokensTest.cpp
        scannerTest/FiniteAutomatonTest.cpp
        scannerTest/LexFileScannerReaderTest.cpp
        scannerTest/LexicalSessionTest.cpp
        scannerTest/TokenFilterTest.cpp
        scannerTest/StateTest.cpp
        scannerTest/TokenMatcher.h
        )
target_link_libraries(scannerTest PRIVATE GTest::gmock_main scanner testUtil)
add_test(NAME scannerTest COMMAND scannerTest)


add_executable(semantic_analyzerTest
        semantic_analyzerTest/SymbolTableTest.cpp
        )
target_link_libraries(semantic_analyzerTest PRIVATE GTest::gmock_main semantic_analyzer)
add_test(NAME semantic_analyzerTest COMMAND semantic_analyzerTest)


add_executable(typesTest
    typesTest/PrimitiveTest.cpp
    typesTest/TypeTest.cpp
    typesTest/FunctionTest.cpp
    typesTest/TypeQueryTest.cpp
    typesTest/ObjectAbiTest.cpp
    typesTest/SysVClassifyTest.cpp
    )
target_link_libraries(typesTest PRIVATE GTest::gmock_main types)
add_test(NAME typesTest COMMAND typesTest)

add_executable(symbolsTest
    symbolsTest/AnnotationStoreTest.cpp
    symbolsTest/FunctionEntryTest.cpp
    symbolsTest/LabelEntryTest.cpp
)
target_link_libraries(symbolsTest PRIVATE GTest::gmock_main symbols)
add_test(NAME symbolsTest COMMAND symbolsTest)



add_executable(translation_unitTest
        translation_unitTest/TranslationUnitTest.cpp
        )
target_link_libraries(translation_unitTest PRIVATE GTest::gmock_main translation_unit testUtil)
add_test(NAME translation_unitTest COMMAND translation_unitTest)


add_executable(codegenTest
        codegenTest/ATandTInstructionSetTests.cpp
        codegenTest/StackMachineTests.cpp
        codegenTest/SysVCallConvTest.cpp
        codegenTest/InstructionIrTest.cpp
        codegenTest/IrPassesTest.cpp
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
        utilTest/StringLiteralDecodeTest.cpp
        utilTest/ImmediateFormatTest.cpp
        )
target_link_libraries(utilTest PRIVATE GTest::gmock_main util)
add_test(NAME utilTest COMMAND utilTest)


add_executable(functionalTest
        functionalTest/TestFixtures.h
        functionalTest/TestFixtures.cpp
        functionalTest/FunctionalTestMain.cpp
        functionalTest/CompilerInitTest.cpp
        functionalTest/ConditionalTest.cpp
        functionalTest/TernaryTest.cpp
        functionalTest/BasicMathTest.cpp
        functionalTest/IncrementsDecrementsTest.cpp
        functionalTest/PointersTest.cpp
        functionalTest/VoidTest.cpp
        functionalTest/InputOutputTest.cpp
        functionalTest/ForLoopTest.cpp
        functionalTest/WhileLoopTest.cpp
        functionalTest/DoWhileTest.cpp
        functionalTest/GotoTest.cpp
        functionalTest/SwitchTest.cpp
        functionalTest/SizeofTest.cpp
        functionalTest/ArraysTest.cpp
        functionalTest/TypeCastTest.cpp
        functionalTest/StructsTest.cpp
        functionalTest/StructsReturnTest.cpp
        functionalTest/IntegerTypesTest.cpp
        functionalTest/FloatingTypesTest.cpp
        functionalTest/UnionsTest.cpp
        functionalTest/EnumsTest.cpp
        functionalTest/TypedefTest.cpp
        functionalTest/ForwardDeclTest.cpp
        functionalTest/AnonymousMembersTest.cpp
        functionalTest/NestedInitTest.cpp
        functionalTest/DesignatedInitTest.cpp
        functionalTest/DesignatedInitUnionTest.cpp
        functionalTest/DesignatedInitResumeTest.cpp
        functionalTest/FlatCurrentObjectInitTest.cpp
        functionalTest/GlobalMultiWordInitTest.cpp
        functionalTest/InitializersTest.cpp
        functionalTest/FunctionPointersTest.cpp
        functionalTest/FactorialTest.cpp
        functionalTest/FibonacciTest.cpp
        functionalTest/FunctionCallTest.cpp
        functionalTest/GccESurfaceTest.cpp
        functionalTest/GenericTest.cpp
        functionalTest/TypeofTest.cpp
        functionalTest/VariadicTest.cpp
        functionalTest/LogicalOperatorsTest.cpp
        functionalTest/UnaryOperatorsTest.cpp
        functionalTest/ShiftOperatorsTest.cpp
        functionalTest/AssignmentOperatorsTest.cpp
        functionalTest/SemanticErrorsTest.cpp
        functionalTest/LanguageSurfaceTest.cpp
        functionalTest/ControlFlowTest.cpp
        functionalTest/ExpressionTest.cpp
        functionalTest/ScopeTest.cpp
        functionalTest/FunctionScopeStaticTest.cpp
        functionalTest/IntegrationTest.cpp
        functionalTest/GlobalVariablesTest.cpp
        functionalTest/HelloStdioTest.cpp
        functionalTest/PieExecutableTest.cpp
        functionalTest/MultiTuTest.cpp
        functionalTest/SysVAbiInteropTest.cpp
        functionalTest/FuzzCrashRegressionTest.cpp
        )
target_link_libraries(functionalTest PRIVATE GTest::gmock driver translation_unit scanner util parser testUtil)
add_dependencies(functionalTest trans)
target_compile_definitions(functionalTest PRIVATE "TRANS_BINARY=\"$<TARGET_FILE:trans>\"")

# Dialect x gtest shard matrix (TRANS_ASM_DIALECT). Override: -DFUNCTIONAL_TEST_SHARDS=16
set(FUNCTIONAL_TEST_SHARDS 8 CACHE STRING "Number of gtest shards for functionalTest (ctest -j runs them in parallel)")
if(NOT FUNCTIONAL_TEST_SHARDS MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "FUNCTIONAL_TEST_SHARDS must be a positive integer (got: ${FUNCTIONAL_TEST_SHARDS})")
endif()
math(EXPR functional_last_shard "${FUNCTIONAL_TEST_SHARDS} - 1")
foreach(dialect intel att)
  foreach(shard RANGE 0 ${functional_last_shard})
    set(test_name functionalTest_${dialect}_shard${shard})
    add_test(NAME ${test_name} COMMAND functionalTest)
    set_tests_properties(${test_name} PROPERTIES
      LABELS "functional;${dialect}"
      ENVIRONMENT "TRANS_ASM_DIALECT=${dialect};GTEST_TOTAL_SHARDS=${FUNCTIONAL_TEST_SHARDS};GTEST_SHARD_INDEX=${shard}"
    )
  endforeach()
endforeach()
