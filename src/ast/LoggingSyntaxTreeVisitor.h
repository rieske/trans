#ifndef _LOGGING_SYNTAX_TREE_VISITOR
#define _LOGGING_SYNTAX_TREE_VISITOR

#include "parser/SyntaxTreeVisitor.h"
#include "parser/ParseTree.h"
#include "AbstractSyntaxTree.h"

#include <string>

namespace ast {

class LoggingSyntaxTreeVisitor : public parser::SyntaxTreeVisitor {
  public:
    LoggingSyntaxTreeVisitor(std::string sourceFileName);
    virtual ~LoggingSyntaxTreeVisitor();

    virtual void visit(AbstractSyntaxTree& ast);
    virtual void visit(parser::ParseTree& parseTree);

  private:
    std::string sourceFileName;
};

} // namespace ast

#endif // _LOGGING_SYNTAX_TREE_VISITOR
