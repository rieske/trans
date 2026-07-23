add_library(semantic_analyzer
        ArrayDecay.cpp
        BuiltinAnalysis.cpp
        ConstantAddress.cpp
        DeclarationAnalyzer.cpp
        InitializerLowering.cpp
        InitializerLoweringFields.cpp
        InitializerLoweringData.cpp
        InitializerLoweringPlacement.cpp
        FunctionEntry.cpp
        LabelEntry.cpp
        SemanticAnalysisVisitor.cpp
        SemanticAnalysisVisitor_Expressions.cpp
        SemanticAnalysisVisitor_Calls.cpp
        SemanticAnalysisVisitor_Statements.cpp
        SemanticAnalyzer.cpp
        SemanticXmlOutputVisitor.cpp
        SymbolTable.cpp
        ValueEntry.cpp
        ValueScope.cpp
        ArrayDecay.h
        BuiltinAnalysis.h
        ConstantAddress.h
        DeclarationAnalyzer.h
        InitializerLowering.h
        FunctionEntry.h
        LabelEntry.h
        SemanticAnalysisVisitor.h
        SemanticAnalysisVisitorInternal.h
        SemanticAnalyzer.h
        SemanticXmlOutputVisitor.h
        SymbolTable.h
        ValueEntry.h
        ValueScope.h)

target_include_directories(semantic_analyzer PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(semantic_analyzer PUBLIC translation_unit types ast util)
