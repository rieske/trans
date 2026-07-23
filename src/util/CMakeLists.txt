add_library(util
        Logger.cpp
        LogManager.cpp
        Logger.h
        LogManager.h
        NullStream.h
        NullStream.cpp
        ProductApprox.h
        StringLiteralDecode.cpp
        StringLiteralDecode.h
        )

target_include_directories(util PUBLIC "${CMAKE_SOURCE_DIR}/src")
