add_library(types
        Type.h
        Type.cpp
        Primitive.h
        Primitive.cpp
        Function.h
        Function.cpp
        ObjectAbi.h
        TypeQuery.h
        TypeQuery.cpp
        )

trans_target_defaults(types)

target_link_libraries(types PUBLIC util)
