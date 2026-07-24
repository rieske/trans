cmake_minimum_required(VERSION 3.17)
project(trans
  VERSION 0.1.0
  DESCRIPTION "Compiler for a C-like programming language"
  LANGUAGES CXX
)

include(CTest)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Parallelism for the day-to-day path lives in the root Makefile (-j$(JOBS)).
# Do not bake -j into CMAKE_CTEST_ARGUMENTS: coverage must run serial (gcov).
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")

# --- Shared target defaults (idiomatic: no global CMAKE_CXX_FLAGS* mutation) ---

add_library(trans_src INTERFACE)
target_include_directories(trans_src INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)

add_library(trans_common INTERFACE)
target_compile_features(trans_common INTERFACE cxx_std_20)
target_compile_options(trans_common INTERFACE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:
    -Wall;-Wextra;-Wno-unused-parameter;-Werror;-pedantic
  >
)

option(TRANS_ENABLE_COVERAGE "Enable gcov/lcov instrumentation (--coverage)" OFF)
add_library(trans_coverage INTERFACE)
if(TRANS_ENABLE_COVERAGE)
  target_compile_options(trans_coverage INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:--coverage>
  )
  target_link_options(trans_coverage INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:--coverage>
  )
  # Prefer foo.gcno over foo.cpp.gcno for lcov path matching.
  set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

# Apply include root + warnings (+ optional coverage) to project targets.
# PUBLIC so static-lib consumers (trans, tests) inherit includes, -Werror, and
# --coverage link flags required for instrumented objects.
function(trans_target_defaults target)
  target_link_libraries(${target} PUBLIC trans_src trans_common)
  if(TRANS_ENABLE_COVERAGE)
    target_link_libraries(${target} PUBLIC trans_coverage)
  endif()
endfunction()

add_subdirectory(src/translation_unit)
add_subdirectory(src/types)
add_subdirectory(src/symbols)
add_subdirectory(src/util)
add_subdirectory(src/scanner)
add_subdirectory(src/parser)
add_subdirectory(src/ast)
add_subdirectory(src/semantic_analyzer)
add_subdirectory(src/codegen)
add_subdirectory(src/driver)

add_executable(trans src/trans.cpp)
trans_target_defaults(trans)
# driver PUBLIC-pulls the rest of the compile pipeline (ast, backend, util, …).
target_link_libraries(trans PRIVATE driver)

# Thread-local va_list save-area helpers (linked into pure-trans user binaries).
# Host C: pure-trans has no _Thread_local.
#
# Single compile: custom_command produces build/va_tls.o (HostToolchain path).
# STATIC library with EXTERNAL_OBJECT pulls that same .o so driverTest links a
# real CMake target (subdirectory absolute-path .o deps break parallel Make).
enable_language(C)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/va_tls.o
    COMMAND ${CMAKE_C_COMPILER} -c -O2 -std=c11 -fno-pic
            -o ${CMAKE_BINARY_DIR}/va_tls.o
            ${CMAKE_SOURCE_DIR}/runtime/va_tls.c
    DEPENDS ${CMAKE_SOURCE_DIR}/runtime/va_tls.c
    COMMENT "Building va_tls.o (thread-local va_list helpers)"
    VERBATIM
)
add_library(va_tls STATIC ${CMAKE_BINARY_DIR}/va_tls.o)
set_source_files_properties(${CMAKE_BINARY_DIR}/va_tls.o PROPERTIES
    EXTERNAL_OBJECT TRUE
    GENERATED TRUE
)
set_target_properties(va_tls PROPERTIES LINKER_LANGUAGE C)
add_dependencies(trans va_tls)

if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(googletest)
  add_subdirectory(test/src)
endif()

# Coverage: project sources only (Make and Ninja). Prefer Debug + TRANS_ENABLE_COVERAGE.
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
  # Serial ctest: parallel functionalTest shards race on shared .gcda files.
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -j1
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage-report
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Run tests (serial for gcov) and produce coverage/lcov.info"
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
