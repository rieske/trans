add_library(driver
        Compiler.cpp
        CompilerComponentsFactory.cpp
        ConfigurationParser.cpp
        Driver.cpp
        Compiler.h
        CompilerComponentsFactory.h
        Configuration.h
        ConfigurationParser.h
        Driver.h)
    #include_directories("${PROJECT_SOURCE_DIR}/src/parser")

target_link_libraries(driver ast codegen)
