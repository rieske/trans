cmake_minimum_required(VERSION 3.6)
project(trans)

set(CMAKE_CXX_STANDARD 14)

################################
# GTest
################################
#ADD_SUBDIRECTORY (libs/googletest)
#enable_testing()
#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
#
#################################
## Unit Tests
#################################
## Add test cpp file
#add_executable( runUnitTests testgtest.cpp )
## Link test executable against gtest & gtest_main
#target_link_libraries(runUnitTests gtest gtest_main)
#add_test( runUnitTests runUnitTests )
#

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



# test/src/astTest/cpp/DeclarationSpecifiersTest.cpp
# test/src/astTest/cpp/DeclarationTest.cpp
# test/src/astTest/cpp/gtest.cpp

#test/src/parserTest/AcceptActionTest.cpp
#test/src/parserTest/BNFFileGrammarTest.cpp
#test/src/parserTest/CanonicalCollectionTest.cpp
#test/src/parserTest/ClosureTest.cpp
#test/src/parserTest/FilePersistedParsingTableTest.cpp
#test/src/parserTest/FirstTableTest.cpp
#test/src/parserTest/gtest.cpp
#test/src/parserTest/LR1ItemTest.cpp
#test/src/parserTest/LR1ParserGeneratedTableTests.cpp
#test/src/parserTest/LR1ParserTest.cpp
#test/src/parserTest/ParseTreeNodeTest.cpp
#test/src/parserTest/ParseTreeTest.cpp
#test/src/parserTest/ParseTreeToSourceConverterTest.cpp
#test/src/parserTest/ProductionTest.cpp
#test/src/parserTest/ShiftActionTest.cpp
#test/src/parserTest/TerminalNodeTest.cpp
#test/src/parserTest/TokenMatcher.h
#test/src/parserTest/TokenStreamTest.cpp
#test/src/parserTest/XmlOutputVisitorTest.cpp
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
#test/src/testUtils/ResourceHelpers.cpp
#test/src/testUtils/ResourceHelpers.h
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