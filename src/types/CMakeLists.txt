add_library(types
        Type.cpp
        TypeCompatibility.cpp
        TypeLayout.cpp
        TypeQuery.cpp
        TypeRecords.cpp
        IntegerConstant.cpp
        Operator.cpp
        SysVClassify.cpp
        Primitive.cpp
        Function.cpp
        )

trans_target_defaults(types)
