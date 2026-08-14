add_library(driver
        Compiler.cpp
        CompilerComponentsFactory.cpp
        ConfigurationParser.cpp
        Driver.cpp
        LanguageFrontEnd.cpp
        ResourcesLocation.cpp
        Compiler.h
        CompilerComponentsFactory.h
        Configuration.h
        Configuration.cpp
        ConfigurationParser.h
        Driver.h
        LanguageFrontEnd.h
        ResourcesLocation.h)

trans_target_defaults(driver)
target_link_libraries(driver PUBLIC ast backend util semantic_analyzer)
