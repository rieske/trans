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
        astTest/DoubleOperandConstantFoldTest.cpp
        astTest/ExpressionTypeTest.cpp
        astTest/FunctionCallBuiltinTypeArgTest.cpp
        astTest/FunctionDefinitionTest.cpp
        astTest/ParseEnvironmentTest.cpp
        astTest/TypeNameTest.cpp
        astTest/TypeSpecifierTest.cpp
        )
target_link_libraries(astTest PRIVATE GTest::gmock_main ast translation_unit semantic_analyzer scanner)
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
        parserTest/ParsingTableFileTest.cpp
        parserTest/ParsingTableTest.cpp
        parserTest/LR1ParserTest.cpp
        parserTest/ProductionTest.cpp
        parserTest/ShiftActionTest.cpp
        parserTest/TokenStreamTest.cpp
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
        )
target_link_libraries(scannerTest PRIVATE GTest::gmock_main scanner testUtil)
add_test(NAME scannerTest COMMAND scannerTest)


add_executable(semantic_analyzerTest
        semantic_analyzerTest/SymbolTableTest.cpp
        semantic_analyzerTest/ValueScopeTest.cpp
        semantic_analyzerTest/InitializerLoweringTest.cpp
        )
target_link_libraries(semantic_analyzerTest PRIVATE GTest::gmock_main semantic_analyzer)
add_test(NAME semantic_analyzerTest COMMAND semantic_analyzerTest)



add_executable(symbolsTest
        symbolsTest/ValueEntryTest.cpp
        symbolsTest/FunctionEntryTest.cpp
        symbolsTest/LabelEntryTest.cpp
        symbolsTest/AnnotationStoreTest.cpp
        symbolsTest/CallPlanMappingTest.cpp
        symbolsTest/AddressPlanTest.cpp
)
target_link_libraries(symbolsTest PRIVATE GTest::gmock_main symbols builtins)
add_test(NAME symbolsTest COMMAND symbolsTest)

add_executable(typesTest
    typesTest/PrimitiveTest.cpp
    typesTest/TypeLayoutTest.cpp
    typesTest/TypeAssignTest.cpp
    typesTest/TypeCompositeTest.cpp
    typesTest/FunctionTest.cpp
    typesTest/TypeQueryTest.cpp
    typesTest/IntegerConstantTest.cpp
    typesTest/SysVClassifyTest.cpp
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
        codegenTest/InstructionIrTest.cpp
        codegenTest/UnimplementedArrayCodegenTest.cpp
        codegenTest/IrPassesTest.cpp
        codegenTest/IntelInstructionSetTests.cpp
        codegenTest/ObjectAbiTests.cpp
        codegenTest/StackMachineTests.cpp
        codegenTest/FrameLayoutTests.cpp
        codegenTest/FrameSymbolTest.cpp
        codegenTest/SymbolRefsTest.cpp
        codegenTest/RetrieveTest.cpp
        codegenTest/SseTest.cpp
        codegenTest/SysVCallConvTest.cpp
        )
target_link_libraries(codegenTest PRIVATE GTest::gmock_main backend ast)
add_test(NAME codegenTest COMMAND codegenTest)


add_executable(driverTest
        driverTest/ConfigurationParserTest.cpp
        driverTest/DriverExitCodeTest.cpp
        driverTest/HeaderDependencyTest.cpp
        driverTest/LanguageFrontEndTest.cpp
        driverTest/PreprocessSkipTest.cpp
        driverTest/CompilerHostTest.cpp
        driverTest/ResourcesLocationTest.cpp
        )
target_link_libraries(driverTest PRIVATE GTest::gmock_main driver translation_unit scanner util parser testUtil)
target_compile_definitions(driverTest PRIVATE "TRANS_BINARY=\"$<TARGET_FILE:trans>\"")
add_dependencies(driverTest trans)
add_test(NAME driverTest COMMAND driverTest)


add_executable(utilTest
        utilTest/ProcessTest.cpp
        utilTest/StringLiteralDecodeTest.cpp
        utilTest/ImmediateFormatTest.cpp
        utilTest/SourcePathTest.cpp
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
        functionalTest/PointersArithTest.cpp
        functionalTest/VoidTest.cpp
        functionalTest/InputOutputTest.cpp
        functionalTest/ForLoopTest.cpp
        functionalTest/WhileLoopTest.cpp
        functionalTest/DoWhileTest.cpp
        functionalTest/GotoTest.cpp
        functionalTest/SwitchTest.cpp
        functionalTest/SizeofTest.cpp
        functionalTest/VlaTest.cpp
        functionalTest/ArraysTest.cpp
        functionalTest/MultiDimArrayTest.cpp
        functionalTest/GitArrayShapesTest.cpp
        functionalTest/ArrayDecayTest.cpp
        functionalTest/ArrayRedeclTest.cpp
        functionalTest/AsmSymbolNamesTest.cpp
        functionalTest/TypeCastTest.cpp
        functionalTest/FactorialTest.cpp
        functionalTest/FibonacciTest.cpp
        functionalTest/FunctionCallTest.cpp
        functionalTest/GccESurfaceTest.cpp
        functionalTest/BuiltinBswapTest.cpp
        functionalTest/BuiltinCtzllTest.cpp
        functionalTest/BuiltinAllocaTest.cpp
        functionalTest/BuiltinConstantPTest.cpp
        functionalTest/GenericTest.cpp
        functionalTest/TypeofTest.cpp
        functionalTest/TypeofCatalogTest.cpp
        functionalTest/TypesCompatibleTest.cpp
        functionalTest/OffsetofTest.cpp
        functionalTest/PackedAttributeTest.cpp
        functionalTest/VariadicTest.cpp
        functionalTest/LogicalOperatorsTest.cpp
        functionalTest/UnaryOperatorsTest.cpp
        functionalTest/ShiftOperatorsTest.cpp
        functionalTest/IntegerPromotionsTest.cpp
        functionalTest/IntegerMemoryWidthTest.cpp
        functionalTest/Uint32ArithTest.cpp
        functionalTest/AssignmentOperatorsTest.cpp
        functionalTest/SemanticErrorsTest.cpp
        functionalTest/SemanticErrorsMoreTest.cpp
        functionalTest/LanguageSurfaceTest.cpp
        functionalTest/ControlFlowTest.cpp
        functionalTest/ExpressionTest.cpp
        functionalTest/ScopeTest.cpp
        functionalTest/FunctionScopeStaticTest.cpp
        functionalTest/IntegrationTest.cpp
        functionalTest/GlobalVariablesTest.cpp
        functionalTest/HelloStdioTest.cpp
        functionalTest/GlobalOperatorsTest.cpp
        functionalTest/GlobalPointersTest.cpp
        functionalTest/GlobalStaticsAndLinkageTest.cpp
        functionalTest/StructsTest.cpp
        functionalTest/StructsLayoutAndErrorsTest.cpp
        functionalTest/StructsReturnTest.cpp
        functionalTest/StructsInitTest.cpp
        functionalTest/DesignatedInitTest.cpp
        functionalTest/DesignatedInitResumeTest.cpp
        functionalTest/DesignatedInitUnionTest.cpp
        functionalTest/FlatCurrentObjectInitTest.cpp
        functionalTest/GlobalMultiWordInitTest.cpp
        functionalTest/StructsMultiWordTest.cpp
        functionalTest/TypesTest.cpp
        functionalTest/Int128ArithTest.cpp
        functionalTest/StaticInitAddressTest.cpp
        functionalTest/StaticInitScalarTest.cpp
        functionalTest/StaticInitWideIntegerTest.cpp
        functionalTest/CharArrayStringInitTest.cpp
        functionalTest/StringLiteralPoolTest.cpp
        functionalTest/CharEscapeTest.cpp
        functionalTest/FloatingTypesTest.cpp
        functionalTest/BitFieldsTest.cpp
        functionalTest/ComplexTypesTest.cpp
        functionalTest/EnumsTest.cpp
        functionalTest/TypedefTest.cpp
        functionalTest/ForwardDeclTest.cpp
        functionalTest/UnionsTest.cpp
        functionalTest/FunctionPointersTest.cpp
        functionalTest/PreprocessorTest.cpp
        functionalTest/GitCompatMacrosTest.cpp
        functionalTest/ProductContractsTest.cpp
        functionalTest/DriverModesTest.cpp
        functionalTest/MixedDeclarationsTest.cpp
        functionalTest/ArrayDeclaratorExtrasTest.cpp
        functionalTest/SysVAbiLongDoubleTest.cpp
        functionalTest/SysVAbiBitFieldTest.cpp
        functionalTest/SysVAbiComplexTest.cpp
        functionalTest/SysVAbiInt128Test.cpp
        functionalTest/IncompleteExternTest.cpp
        functionalTest/CompoundLiteralTest.cpp
        functionalTest/SysVAbiAggregatesTest.cpp
        functionalTest/SysVAbiNarrowGprTest.cpp
        functionalTest/SysVAbiEnumTest.cpp
        functionalTest/SysVAbiArgPressureTest.cpp
        functionalTest/SysVAbiReturnPressureTest.cpp
        functionalTest/SysVAbiVaPressureTest.cpp
        functionalTest/SysVAbiLayoutTest.cpp
        functionalTest/SysVAbiStackAlignTest.cpp
        functionalTest/SysVAbiPackedTest.cpp
        functionalTest/SysVAbiMatrixTest.cpp
        functionalTest/EmptyTranslationUnitTest.cpp
        functionalTest/MultiTuTest.cpp
        functionalTest/PieExecutableTest.cpp
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
