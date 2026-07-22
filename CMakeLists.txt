cmake_minimum_required(VERSION 3.17)
project(trans)

include(FetchContent)
include(CTest)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Werror -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(src/driver)
add_subdirectory(src/translation_unit)
add_subdirectory(src/scanner)
add_subdirectory(src/semantic_analyzer)
add_subdirectory(src/parser)
add_subdirectory(src/ast)
add_subdirectory(src/types)
add_subdirectory(src/codegen)
add_subdirectory(src/util)

add_executable(trans src/trans.cpp)
target_include_directories(trans PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(trans PRIVATE
  driver
  translation_unit
  scanner
  parser
  ast
  semantic_analyzer
  types
  codegen
  util
)

if(BUILD_TESTING)
  add_subdirectory(test/src)
endif()

# Coverage: project sources only (Make and Ninja). Prefer Debug (gcov flags).
# Capture both source and object trees so --no-external keeps ${CMAKE_SOURCE_DIR}/src
# (object-dir-only capture treats those sources as external and yields an empty report).
# Do not scan _deps/ or test/ — googletest triggers lcov line-table errors.
# Paths in the report are made repo-relative for Coveralls.
# coverage-report: lcov only (assumes tests already ran and wrote .gcda).
# coverage: run tests, then coverage-report.
file(WRITE "${CMAKE_BINARY_DIR}/RelativizeLcov.cmake" [=[
if(NOT EXISTS "${INFO_FILE}")
  message(FATAL_ERROR "lcov info file not found: ${INFO_FILE}")
endif()
file(READ "${INFO_FILE}" contents)
string(REPLACE "SF:${PREFIX}" "SF:" contents "${contents}")
file(WRITE "${INFO_FILE}" "${contents}")
]=])
add_custom_target(coverage-report
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
  COMMAND lcov
          --ignore-errors empty,mismatch,gcov
          --rc geninfo_unexecuted_blocks=1
          --no-external
          -c
          -d ${CMAKE_SOURCE_DIR}/src
          -d ${CMAKE_BINARY_DIR}/src
          -o ${CMAKE_BINARY_DIR}/coverage/lcov.info
  COMMAND ${CMAKE_COMMAND}
          -DINFO_FILE=${CMAKE_BINARY_DIR}/coverage/lcov.info
          -DPREFIX=${CMAKE_SOURCE_DIR}/
          -P ${CMAKE_BINARY_DIR}/RelativizeLcov.cmake
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Produce coverage/lcov.info from existing gcov data"
)
add_custom_target(coverage
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage-report
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Run tests and produce coverage/lcov.info"
  DEPENDS trans
)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES coverage)

add_custom_target(scrub
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/coverage
  COMMAND find ${CMAKE_BINARY_DIR} -type f \\( -name "*.gcda" -o -name "*.gcno" \\) -delete
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Clean build outputs and gcov artifacts"
)
