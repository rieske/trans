add_library(types
        Type.h
        Type.cpp
        TypeLayout.cpp
        TypeQuery.cpp
        TypeQuery.h
        ObjectAbi.h
        ObjectAbiType.h
        SysVClass.h
        SysVClassify.h
        SysVClassify.cpp
        Primitive.h
        Primitive.cpp
        Function.h
        Function.cpp
        )

trans_target_defaults(types)
