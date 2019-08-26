cmake_minimum_required(VERSION 3.6)
project(trans)

set(CMAKE_CXX_STANDARD 14)


ADD_SUBDIRECTORY(libs/gmock)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
enable_testing()


include_directories("${PROJECT_BINARY_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/src/driver")
add_subdirectory(src/driver)

include_directories("${PROJECT_SOURCE_DIR}/src/translation_unit")
add_subdirectory(src/translation_unit)

include_directories("${PROJECT_SOURCE_DIR}/src/scanner")
add_subdirectory(src/scanner)

include_directories("${PROJECT_SOURCE_DIR}/src/parser")
add_subdirectory(src/parser)

include_directories("${PROJECT_SOURCE_DIR}/src/ast")
add_subdirectory(src/ast)

include_directories("${PROJECT_SOURCE_DIR}/src/semantic_analyzer")
add_subdirectory(src/semantic_analyzer)

include_directories("${PROJECT_SOURCE_DIR}/src/types")
add_subdirectory(src/types)

include_directories("${PROJECT_SOURCE_DIR}/src/codegen")
add_subdirectory(src/codegen)

include_directories("${PROJECT_SOURCE_DIR}/src/util")
add_subdirectory(src/util)

add_executable(trans src/trans.cpp)
target_link_libraries(trans driver translation_unit scanner parser ast semantic_analyzer types codegen util)

add_subdirectory(test)


