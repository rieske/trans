#ifndef _SYNTAX_TREE_H_
#define _SYNTAX_TREE_H_

#include <ostream>

#include "SyntaxTreeVisitor.h"

namespace parser {

class SyntaxTree {
public:
    virtual ~SyntaxTree() = default;

    virtual void accept(SyntaxTreeVisitor& visitor) = 0;

    virtual void outputXml(std::ostream& stream) const = 0;
    virtual void outputSource(std::ostream& stream) const = 0;
};

} // namespace parser

#endif // _SYNTAX_TREE_H_

