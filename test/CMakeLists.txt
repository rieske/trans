add_library(testUtil src/util/ResourceHelpers.cpp src/util/ResourceHelpers.h)
target_include_directories(testUtil PUBLIC src/util)


add_executable(astTest src/astTest/gtest.cpp
        src/astTest/DeclarationSpecifiersTest.cpp
        src/astTest/DeclarationTest.cpp)
target_link_libraries(astTest gmock ast translation_unit)
add_test(astTest astTest)

add_executable(parserTest src/parserTest/gtest.cpp
        src/parserTest/AcceptActionTest.cpp
        src/parserTest/BNFFileGrammarTest.cpp
        src/parserTest/CanonicalCollectionTest.cpp
        src/parserTest/ClosureTest.cpp
        src/parserTest/FilePersistedParsingTableTest.cpp
        src/parserTest/FirstTableTest.cpp
        src/parserTest/gtest.cpp
        src/parserTest/LR1ItemTest.cpp
        src/parserTest/LR1ParserGeneratedTableTests.cpp
        src/parserTest/LR1ParserTest.cpp
        src/parserTest/ParseTreeNodeTest.cpp
        src/parserTest/ParseTreeTest.cpp
        src/parserTest/ParseTreeToSourceConverterTest.cpp
        src/parserTest/ProductionTest.cpp
        src/parserTest/ShiftActionTest.cpp
        src/parserTest/TerminalNodeTest.cpp
        src/parserTest/TokenMatcher.h
        src/parserTest/TokenStreamTest.cpp
        src/parserTest/XmlOutputVisitorTest.cpp)
target_link_libraries(parserTest gmock parser driver testUtil)
add_test(parserTest parserTest)


add_executable(scannerTest src/scannerTest/FiniteAutomatonScannerTest.cpp
        src/scannerTest/FiniteAutomatonTest.cpp
        src/scannerTest/gtest.cpp
        src/scannerTest/LexFileFiniteAutomatonTest.cpp
        src/scannerTest/StateTest.cpp
        src/scannerTest/TokenMatcher.h)
target_link_libraries(scannerTest gmock scanner testUtil)
add_test(scannerTest scannerTest)


add_executable(semantic_analyzerTest src/semantic_analyzerTest/gtest.cpp
        src/semantic_analyzerTest/SemanticXmlOutputVisitorTest.cpp)
target_link_libraries(semantic_analyzerTest gmock semantic_analyzer)
add_test(semantic_analyzerTest semantic_analyzerTest)


add_executable(typesTest src/typesTest/gtest.cpp)
target_link_libraries(typesTest gmock types)
add_test(typesTest typesTest)


add_executable(translation_unitTest src/translation_unitTest/gtest.cpp)
target_link_libraries(translation_unitTest gmock translation_unit testUtil)
add_test(translation_unitTest translation_unitTest)


add_executable(codegenTest src/codegenTest/ATandTInstructionSetTests.cpp
        src/codegenTest/gtest.cpp
        src/codegenTest/StackMachineTests.cpp)
target_link_libraries(codegenTest gmock codegen)
add_test(codegenTest codegenTest)


add_executable(driverTest src/driverTest/CompilerTest.cpp
        src/driverTest/ConfigurationParserTest.cpp
        src/driverTest/gtest.cpp
        src/driverTest/TranslationUnitTest.cpp)
target_link_libraries(driverTest gmock driver translation_unit scanner util parser testUtil)
add_test(driverTest driverTest)


