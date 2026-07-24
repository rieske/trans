add_library(scanner
        FiniteAutomaton.h
        FiniteAutomaton.cpp
        LexFileScannerReader.h
        LexFileScannerReader.cpp
        ScannerBuilder.h
        ScannerBuilder.cpp
        Scanner.h
        Scanner.cpp
        State.h
        State.cpp
        Token.h
        Token.cpp
        TypedefRegistry.h
        TypedefRegistry.cpp
        EnumConstantRegistry.h
        EnumConstantRegistry.cpp
        )

trans_target_defaults(scanner)
target_link_libraries(scanner PUBLIC translation_unit util)
