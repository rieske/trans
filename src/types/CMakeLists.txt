add_library(types
        Type.h
        Type.cpp
        Primitive.h
        Primitive.cpp
        Function.h
        Function.cpp
        )

target_include_directories(types PUBLIC "${CMAKE_SOURCE_DIR}/src")
