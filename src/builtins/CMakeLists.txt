add_library(builtins
        BuiltinRegistry.cpp
        BuiltinRegistry.h
)

trans_target_defaults(builtins)
# Leaf for SA: types + symbols::CallPlan (no link cycle with ast).
# FunctionCall.h is header-only for the enum; link types for Type factories.
target_link_libraries(builtins PUBLIC types symbols)
# ast headers need include path; do not link full ast (builtins is a leaf for SA+backend).
target_include_directories(builtins PUBLIC ${CMAKE_SOURCE_DIR}/src)
