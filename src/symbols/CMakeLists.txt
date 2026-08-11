add_library(symbols
        AnnotationStore.cpp
        AnnotationStore.h
        AnnotationTypes.h
        AddressPlan.h
        StaticInit.h
        FunctionEntry.cpp
        FunctionEntry.h
        LabelEntry.cpp
        LabelEntry.h
        ValueEntry.cpp
        ValueEntry.h
        )

trans_target_defaults(symbols)
target_link_libraries(symbols PUBLIC types translation_unit)
