add_library(util
        Logger.cpp
        LogManager.cpp
        Logger.h
        LogManager.h
        ImmediateFormat.h
        FloatingLiteral.h
        NullStream.h
        NullStream.cpp
        Process.cpp
        Process.h
        StringLiteralDecode.cpp
        StringLiteralDecode.h
        SourcePath.h
        Diagnostic.h
        Diagnostic.cpp
        )

trans_target_defaults(util)
target_link_libraries(util PUBLIC translation_unit)
