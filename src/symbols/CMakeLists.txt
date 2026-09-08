add_library(symbols
        AnnotationStore.cpp
        FunctionEntry.cpp
        LabelEntry.cpp
        ValueEntry.cpp
        )

trans_target_defaults(symbols)
target_link_libraries(symbols PUBLIC types translation_unit)
