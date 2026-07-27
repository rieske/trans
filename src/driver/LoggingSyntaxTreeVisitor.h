#ifndef _LOGGING_SYNTAX_TREE_VISITOR
#define _LOGGING_SYNTAX_TREE_VISITOR

#include "parser/SyntaxTreeVisitor.h"
#include "parser/ParseTree.h"
#include "ast/AbstractSyntaxTree.h"

#include <string>

namespace driver {

class LoggingSyntaxTreeVisitor : public parser::SyntaxTreeVisitor {
  public:
    LoggingSyntaxTreeVisitor(std::string sourceFileName);
    virtual ~LoggingSyntaxTreeVisitor();

    virtual void visit(ast::AbstractSyntaxTree& ast);
    virtual void visit(parser::ParseTree& parseTree);

  private:
    std::string sourceFileName;
};

} // namespace driver

#endif // _LOGGING_SYNTAX_TREE_VISITOR
