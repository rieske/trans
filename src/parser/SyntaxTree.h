#ifndef _SYNTAX_TREE_H_
#define _SYNTAX_TREE_H_

#include <iostream>

#include "SyntaxTreeVisitor.h"

namespace semantic_analyzer {
class SemanticAnalyzer;
} /* namespace semantic_analyzer */

namespace parser {

class SyntaxTree {
public:
    virtual ~SyntaxTree() = default;

    virtual void accept(SyntaxTreeVisitor& visitor) = 0;

    virtual void outputXml(std::ostream& stream) const = 0;
    virtual void outputSource(std::ostream& stream) const = 0;
};

}

#endif // _SYNTAX_TREE_H_

