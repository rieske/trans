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

target_link_libraries(driver ast codegen)
