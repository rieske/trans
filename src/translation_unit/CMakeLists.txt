add_library(translation_unit
        Context.cpp
        TranslationUnit.cpp
        Context.h
        TranslationUnit.h)

target_include_directories(translation_unit PUBLIC "${CMAKE_SOURCE_DIR}/src")
