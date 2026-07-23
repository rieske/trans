add_library(driver
        Compiler.cpp
        CompilerComponentsFactory.cpp
        ConfigurationParser.cpp
        Driver.cpp
        Compiler.h
        CompilerComponentsFactory.h
        Configuration.h
        Configuration.cpp
        ConfigurationParser.h
        Driver.h)

target_include_directories(driver PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(driver PUBLIC ast codegen util)
