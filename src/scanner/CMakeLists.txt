add_library(scanner
        FiniteAutomaton.cpp
        FiniteAutomatonScanner.cpp
        LexFileFiniteAutomaton.cpp
        State.cpp
        Token.cpp
        FiniteAutomaton.h
        FiniteAutomatonScanner.h
        LexFileFiniteAutomaton.h
        Scanner.h
        State.h
        Token.h)

target_include_directories(scanner PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(scanner translation_unit)
