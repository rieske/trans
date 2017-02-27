cmake_minimum_required(VERSION 3.6)
project(trans)

set(CMAKE_CXX_STANDARD 14)


ADD_SUBDIRECTORY (libs/gmock)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
enable_testing()


include_directories("${PROJECT_BINARY_DIR}")

include_directories ("${PROJECT_SOURCE_DIR}/src/driver")
add_subdirectory (src/driver)

include_directories ("${PROJECT_SOURCE_DIR}/src/translation_unit")
add_subdirectory (src/translation_unit)

include_directories ("${PROJECT_SOURCE_DIR}/src/scanner")
add_subdirectory (src/scanner)

include_directories ("${PROJECT_SOURCE_DIR}/src/parser")
add_subdirectory (src/parser)

include_directories ("${PROJECT_SOURCE_DIR}/src/ast")
add_subdirectory (src/ast)

include_directories ("${PROJECT_SOURCE_DIR}/src/semantic_analyzer")
add_subdirectory (src/semantic_analyzer)

include_directories ("${PROJECT_SOURCE_DIR}/src/types")
add_subdirectory (src/types)

include_directories ("${PROJECT_SOURCE_DIR}/src/codegen")
add_subdirectory (src/codegen)

include_directories ("${PROJECT_SOURCE_DIR}/src/util")
add_subdirectory (src/util)

add_executable(trans src/trans.cpp)
target_link_libraries(trans driver translation_unit scanner parser ast semantic_analyzer types codegen util)


include_directories ("${PROJECT_SOURCE_DIR}/src/test")

include_directories ("${PROJECT_SOURCE_DIR}/src/test/util")
add_subdirectory (src/test/util)

add_executable(utilTest test/src/utilTest/gtest.cpp)
target_link_libraries(utilTest gmock)
add_test(utilTest utilTest)

#add_executable(astTest test/src/astTest/gtest.cpp
#        test/src/astTest/DeclarationSpecifiersTest.cpp
#        test/src/astTest/DeclarationTest.cpp)
#target_link_libraries(astTest ast semantic_analyzer  types translation_unit gmock)
#add_test(astTest astTest)

add_executable(parserTest test/src/parserTest/gtest.cpp
        test/src/parserTest/AcceptActionTest.cpp
        test/src/parserTest/BNFFileGrammarTest.cpp
        test/src/parserTest/CanonicalCollectionTest.cpp
        test/src/parserTest/ClosureTest.cpp
        test/src/parserTest/FilePersistedParsingTableTest.cpp
        test/src/parserTest/FirstTableTest.cpp
        test/src/parserTest/gtest.cpp
        test/src/parserTest/LR1ItemTest.cpp
        test/src/parserTest/LR1ParserGeneratedTableTests.cpp
        test/src/parserTest/LR1ParserTest.cpp
        test/src/parserTest/ParseTreeNodeTest.cpp
        test/src/parserTest/ParseTreeTest.cpp
        test/src/parserTest/ParseTreeToSourceConverterTest.cpp
        test/src/parserTest/ProductionTest.cpp
        test/src/parserTest/ShiftActionTest.cpp
        test/src/parserTest/TerminalNodeTest.cpp
        test/src/parserTest/TokenMatcher.h
        test/src/parserTest/TokenStreamTest.cpp
        test/src/parserTest/XmlOutputVisitorTest.cpp)
target_link_libraries(parserTest gmock parser)
add_test(parserTest parserTest)

#
#


#
#test/src/scannerTest/FiniteAutomatonScannerTest.cpp
#test/src/scannerTest/FiniteAutomatonTest.cpp
#test/src/scannerTest/gtest.cpp
#test/src/scannerTest/LexFileFiniteAutomatonTest.cpp
#test/src/scannerTest/StateTest.cpp
#test/src/scannerTest/TokenMatcher.h
#
#test/src/semantic_analyzerTest/gtest.cpp
#test/src/semantic_analyzerTest/SemanticXmlOutputVisitorTest.cpp
#test/src/util/ResourceHelpers.cpp
#test/src/util/ResourceHelpers.h
#test/src/testUtilsTest/gtest.cpp
#
#test/src/utilTest/gtest.cpp
#
#test/src/typesTest/gtest.cpp
#
#test/src/translation_unitTest/gtest.cpp
#
#test/src/codegenTest/ATandTInstructionSetTests.cpp
#test/src/codegenTest/gtest.cpp
#test/src/codegenTest/StackMachineTests.cpp
#
#test/src/driverTest/CompilerTest.cpp
#test/src/driverTest/ConfigurationParserTest.cpp
#test/src/driverTest/gtest.cpp
#test/src/driverTest/TranslationUnitTest.cpp