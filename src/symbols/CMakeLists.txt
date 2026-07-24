add_library(symbols
        ValueEntry.cpp
        LabelEntry.cpp
        FunctionEntry.cpp
        AnnotationStore.cpp
        ValueEntry.h
        LabelEntry.h
        FunctionEntry.h
        AnnotationStore.h
)

trans_target_defaults(symbols)
target_link_libraries(symbols PUBLIC translation_unit types)
