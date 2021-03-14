add_library(scanner
        FiniteAutomaton.h
        FiniteAutomaton.cpp
        LexFileFiniteAutomaton.h
        LexFileFiniteAutomaton.cpp
        Scanner.h
        Scanner.cpp
        State.h
        State.cpp
        Token.h
        Token.cpp
        )

target_include_directories(scanner PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(scanner translation_unit)
