add_library(symbols
        AnnotationStore.cpp
        AnnotationStore.h
        AddressPlan.h
        ValueEntry.cpp
        ValueEntry.h
        )

trans_target_defaults(symbols)
target_link_libraries(symbols PUBLIC types translation_unit)
