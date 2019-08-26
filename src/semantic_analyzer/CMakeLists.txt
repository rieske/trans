add_library(semantic_analyzer
        FunctionEntry.cpp
        LabelEntry.cpp
        SemanticAnalysisVisitor.cpp
        SemanticAnalyzer.cpp
        SemanticXmlOutputVisitor.cpp
        SymbolTable.cpp
        ValueEntry.cpp
        ValueScope.cpp
        FunctionEntry.h
        LabelEntry.h
        SemanticAnalysisVisitor.h
        SemanticAnalyzer.h
        SemanticXmlOutputVisitor.h
        SymbolTable.h
        ValueEntry.h
        ValueScope.h)

target_link_libraries(semantic_analyzer ast)
