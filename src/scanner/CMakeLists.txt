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
        TokenFilter.h
        TokenFilter.cpp
        IdentifierTable.h
        IdentifierTable.cpp
        ParseTypeTable.h
        ParseTypeTable.cpp
        EnumConstantRegistry.h
        EnumConstantRegistry.cpp
        LexicalSession.h
        )

trans_target_defaults(scanner)
target_link_libraries(scanner PUBLIC translation_unit types util)
