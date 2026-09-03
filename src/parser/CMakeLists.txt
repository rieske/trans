add_library(parser_core
        Action.cpp
        BNFFileReader.cpp
        CanonicalCollection.cpp
        FirstTable.cpp
        GenerateParsingTable.cpp
        Grammar.cpp
        GrammarBuilder.cpp
        LookaheadActionTable.cpp
        LR1Item.cpp
        LR1Parser.cpp
        ParsingTable.cpp
        CompileParsingTable.cpp
        Production.cpp
        TokenStream.cpp
        Action.h
        BNFFileReader.h
        CanonicalCollection.h
        CompileParsingTable.h
        FirstTable.h
        GenerationBuffers.h
        GenerateParsingTable.h
        HashCombine.h
        Grammar.h
        GrammarBuilder.h
        LookaheadActionTable.h
        LR1Item.h
        LR1Parser.h
        Parser.h
        ParseExtensions.h
        ParsingTable.h
        Production.h
        SyntaxTreeBuilder.h
        SyntaxTreeBuilder.cpp
        TokenStream.h)

trans_target_defaults(parser_core)
target_link_libraries(parser_core PUBLIC scanner util)

add_executable(parsingTableGenerator ParsingTableGenerator.cpp)
trans_target_defaults(parsingTableGenerator)
target_link_libraries(parsingTableGenerator PRIVATE parser_core)

set(generated_table_source ${CMAKE_CURRENT_BINARY_DIR}/productParsingTable.cpp)
add_custom_command(
  OUTPUT ${generated_table_source}
  COMMAND parsingTableGenerator
          ${CMAKE_SOURCE_DIR}/resources/configuration/grammar.bnf
          ${generated_table_source}
  DEPENDS parsingTableGenerator
          ${CMAKE_SOURCE_DIR}/resources/configuration/grammar.bnf
  COMMENT "Generate compiled parsing table"
  VERBATIM
)

add_library(parser ${generated_table_source})
trans_target_defaults(parser)
target_link_libraries(parser PUBLIC parser_core)
