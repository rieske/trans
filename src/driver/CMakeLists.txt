add_library(driver
        Compiler.cpp
        ConfigurationParser.cpp
        Driver.cpp
        LanguageFrontEnd.cpp
        ResourcesLocation.cpp
        Configuration.cpp)

trans_target_defaults(driver)
target_link_libraries(driver PUBLIC ast backend util semantic_analyzer)
