add_library(util
        Logger.cpp
        LogManager.cpp
        Logger.h
        LogManager.h
        NullStream.h
        NullStream.cpp
        Process.cpp
        Process.h
        ProductApprox.h
        StringLiteralDecode.cpp
        StringLiteralDecode.h
        )

trans_target_defaults(util)
