add_library(types
        Type.cpp
        TypeLayout.cpp
        TypeQuery.cpp
        IntegerConstant.cpp
        Operator.cpp
        SysVClassify.cpp
        Primitive.cpp
        Function.cpp
        )

trans_target_defaults(types)
