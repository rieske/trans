add_library(driver
        Compiler.cpp
        CompilerComponentsFactory.cpp
        ConfigurationParser.cpp
        Driver.cpp
        HostToolchain.cpp
        LoggingSyntaxTreeVisitor.cpp
        VerboseSyntaxTreeBuilder.cpp
        Compiler.h
        CompilerComponentsFactory.h
        Configuration.h
        Configuration.cpp
        ConfigurationParser.h
        Driver.h
        HostToolchain.h)

trans_target_defaults(driver)
target_link_libraries(driver PUBLIC ast backend util semantic_analyzer)
