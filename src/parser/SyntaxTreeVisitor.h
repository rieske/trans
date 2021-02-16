#ifndef SYNTAXTREEVISITOR_H_
#define SYNTAXTREEVISITOR_H_

namespace ast {
class AbstractSyntaxTree;
}

namespace parser {

class ParseTree;

class SyntaxTreeVisitor {
public:
    virtual ~SyntaxTreeVisitor() = default;

    virtual void visit(ast::AbstractSyntaxTree& tree) = 0;
    virtual void visit(ParseTree& parseTree) = 0;
};

} // namespace parser

#endif // SYNTAXTREEVISITOR_H_
