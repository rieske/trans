add_library(util
        Logger.cpp
        LogManager.cpp
        NullStream.cpp
        Process.cpp
        StringLiteralDecode.cpp
        Diagnostic.cpp
        )

trans_target_defaults(util)
target_link_libraries(util PUBLIC translation_unit)
