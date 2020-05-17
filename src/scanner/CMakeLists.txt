add_library(scanner
        EOLCommentState.cpp
        FiniteAutomaton.cpp
        FiniteAutomatonScanner.cpp
        IdentifierState.cpp
        LexFileFiniteAutomaton.cpp
        State.cpp
        StringLiteralState.cpp
        Token.cpp
        EOLCommentState.h
        FiniteAutomaton.h
        FiniteAutomatonScanner.h
        IdentifierState.h
        LexFileFiniteAutomaton.h
        Scanner.h
        State.h
        StringLiteralState.h
        Token.h)

target_link_libraries(scanner translation_unit)
target_include_directories(scanner PUBLIC "${CMAKE_SOURCE_DIR}/src")
#target_include_directories(scanner PRIVATE ${YOUR_DIRECTORY})

