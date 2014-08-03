#ifndef _SYNTAX_TREE_H_
#define _SYNTAX_TREE_H_

#include <iostream>

namespace semantic_analyzer {
class SemanticAnalyzer;
} /* namespace semantic_analyzer */

class SymbolTable;

namespace parser {

class SyntaxTree {
public:
    virtual ~SyntaxTree() {
    }

    virtual void analyzeWith(semantic_analyzer::SemanticAnalyzer& semanticAnalyzer) = 0;

    virtual void outputXml(std::ostream& stream) const = 0;
    virtual void outputSource(std::ostream& stream) const = 0;
};

}

#endif // _SYNTAX_TREE_H_

