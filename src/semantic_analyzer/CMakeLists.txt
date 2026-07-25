add_library(semantic_analyzer
        FunctionEntry.cpp
        InitializerLowering.cpp
        LabelEntry.cpp
        SemanticAnalysisVisitor.cpp
        SemanticAnalysisVisitor_Calls.cpp
        SemanticAnalysisVisitor_Expressions.cpp
        SemanticAnalysisVisitor_Statements.cpp
        SemanticAnalyzer.cpp
        SemanticXmlOutputVisitor.cpp
        SymbolTable.cpp
        ValueEntry.cpp
        ValueScope.cpp
        FunctionEntry.h
        LabelEntry.h
        SemanticAnalysisVisitor.h
        SemanticAnalysisVisitorInternal.h
        SemanticAnalyzer.h
        SemanticXmlOutputVisitor.h
        SymbolTable.h
        ValueEntry.h
        ValueScope.h)

trans_target_defaults(semantic_analyzer)
target_link_libraries(semantic_analyzer PUBLIC translation_unit types ast symbols)
