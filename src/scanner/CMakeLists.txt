add_library(scanner
        FiniteAutomaton.cpp
        LexFileScannerReader.cpp
        ScannerBuilder.cpp
        Scanner.cpp
        State.cpp
        Token.cpp
        TokenFilter.cpp
        IdentifierTable.cpp
        ParseTypeTable.cpp
        EnumConstantRegistry.cpp
        )

trans_target_defaults(scanner)
target_link_libraries(scanner PUBLIC translation_unit types util)
