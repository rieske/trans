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
        )

trans_target_defaults(util)
