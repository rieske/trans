add_library(util
        Logger.cpp
        LogManager.cpp
        Logger.h
        LogManager.h
        ImmediateFormat.h
        FloatingBits.h
        FloatingLiteral.h
        NullStream.h
        NullStream.cpp
        Process.cpp
        Process.h
        StringLiteralDecode.cpp
        StringLiteralDecode.h
        SourcePath.h
        )

trans_target_defaults(util)
