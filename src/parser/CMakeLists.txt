add_library(parser
        Action.cpp
        BNFFileReader.cpp
        CanonicalCollection.cpp
        Closure.cpp
        FilePersistedParsingTable.cpp
        FirstTable.cpp
        GeneratedParsingTable.cpp
        GoTo.cpp
        Grammar.cpp
        GrammarBuilder.cpp
        LALR1Strategy.cpp
        LookaheadActionTable.cpp
        LR1Item.cpp
        LR1Parser.cpp
        LR1Strategy.cpp
        ParseTree.cpp
        ParseTreeBuilder.cpp
        ParseTreeNode.cpp
        ParseTreeToSourceConverter.cpp
        ParsingTable.cpp
        ParsingTableReader.cpp
        Production.cpp
        TerminalNode.cpp
        TokenStream.cpp
        XmlOutputVisitor.cpp
        Action.h
        BNFFileReader.h
        CanonicalCollection.h
        CanonicalCollectionStrategy.h
        Closure.h
        FilePersistedParsingTable.h
        FirstTable.h
        GeneratedParsingTable.h
        GoTo.h
        Grammar.h
        GrammarBuilder.h
        LALR1Strategy.h
        LookaheadActionTable.h
        LR1Item.h
        LR1Parser.h
        LR1Strategy.h
        Parser.h
        ParseTree.h
        ParseTreeBuilder.h
        ParseTreeNode.h
        ParseTreeNodeVisitor.h
        ParseTreeToSourceConverter.h
        ParsingTable.h
        ParsingTableReader.h
        Production.h
        SyntaxTree.h
        SyntaxTreeBuilder.h
        SyntaxTreeBuilder.cpp
        SyntaxTreeVisitor.h
        TerminalNode.h
        TokenStream.h
        XmlOutputVisitor.h)

target_include_directories(parser PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(parser scanner util)
