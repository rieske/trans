cmake_minimum_required(VERSION 3.17)
project(trans)

include(FetchContent)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Werror -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0
)

FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

add_subdirectory(test/src)
enable_testing()

add_executable(trans src/trans.cpp)

add_subdirectory(src/driver)
set(DRIVER_OBJECTS ${CMAKE_BINARY_DIR}/src/driver/CMakeFiles/driver.dir)

add_subdirectory(src/translation_unit)
set(TRANSLATION_UNIT_OBJECTS ${CMAKE_BINARY_DIR}/src/translation_unit/CMakeFiles/translation_unit.dir)

add_subdirectory(src/scanner)
set(SCANNER_OBJECTS ${CMAKE_BINARY_DIR}/src/scanner/CMakeFiles/scanner.dir)

add_subdirectory(src/semantic_analyzer)
set(SEMANTIC_ANALYZER_OBJECTS ${CMAKE_BINARY_DIR}/src/semantic_analyzer/CMakeFiles/semantic_analyzer.dir)

add_subdirectory(src/parser)
set(PARSER_OBJECTS ${CMAKE_BINARY_DIR}/src/parser/CMakeFiles/parser.dir)

add_subdirectory(src/ast)
set(AST_OBJECTS ${CMAKE_BINARY_DIR}/src/ast/CMakeFiles/ast.dir)

add_subdirectory(src/types)
set(TYPES_OBJECTS ${CMAKE_BINARY_DIR}/src/types/CMakeFiles/types.dir)

add_subdirectory(src/codegen)
set(CODEGEN_OBJECTS ${CMAKE_BINARY_DIR}/src/codegen/CMakeFiles/codegen.dir)

add_subdirectory(src/util)
set(UTIL_OBJECTS ${CMAKE_BINARY_DIR}/src/util/CMakeFiles/util.dir)

target_link_libraries(trans driver translation_unit scanner parser ast semantic_analyzer types codegen util)


add_custom_target(coverage
    COMMAND mkdir -p coverage
    COMMAND ${CMAKE_MAKE_PROGRAM} test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
add_custom_command(TARGET coverage
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/driver" -d ${DRIVER_OBJECTS} -o driver.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/translation_unit" -d ${TRANSLATION_UNIT_OBJECTS} -o translation_unit.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/scanner" -d ${SCANNER_OBJECTS} -o scanner.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/parser" -d ${PARSER_OBJECTS} -o parser.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/ast" -d ${AST_OBJECTS} -o ast.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/semantic_analyzer" -d ${SEMANTIC_ANALYZER_OBJECTS} -o semantic_analyzer.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/types" -d ${TYPES_OBJECTS} -o types.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/codegen" -d ${CODEGEN_OBJECTS} -o codegen.info
    COMMAND lcov --no-external -c -d "${CMAKE_SOURCE_DIR}/src/util" -d ${UTIL_OBJECTS} -o util.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
    )
add_dependencies(coverage trans)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)

add_custom_target(scrub
    COMMAND ${CMAKE_MAKE_PROGRAM} clean
    COMMAND rm -f ${DRIVER_OBJECTS}/*.gcno
    COMMAND rm -f ${DRIVER_OBJECTS}/*.gcno
    COMMAND rm -f ${TRANSLATION_UNIT_OBJECTS}/*.gcno
    COMMAND rm -f ${TRANSLATION_UNIT_OBJECTS}/*.gcno
    COMMAND rm -f ${SCANNER_OBJECTS}/*.gcno
    COMMAND rm -f ${SCANNER_OBJECTS}/*.gcno
    COMMAND rm -f ${PARSER_OBJECTS}/*.gcno
    COMMAND rm -f ${PARSER_OBJECTS}/*.gcno
    COMMAND rm -f ${AST_OBJECTS}/*.gcda
    COMMAND rm -f ${AST_OBJECTS}/*.gcda
    COMMAND rm -f ${SEMANTIC_ANALYZER_OBJECTS}/*.gcda
    COMMAND rm -f ${SEMANTIC_ANALYZER_OBJECTS}/*.gcda
    COMMAND rm -f ${TYPES_OBJECTS}/*.gcno
    COMMAND rm -f ${TYPES_OBJECTS}/*.gcda
    COMMAND rm -f ${CODEGEN_OBJECTS}/*.gcda
    COMMAND rm -f ${CODEGEN_OBJECTS}/*.gcda
    COMMAND rm -f ${UTIL_OBJECTS}/*.gcda
    COMMAND rm -f ${UTIL_OBJECTS}/*.gcda
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

