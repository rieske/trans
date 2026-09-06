#ifndef LABELEDSTATEMENT_H_
#define LABELEDSTATEMENT_H_

#include <memory>
#include <string>

#include "ast/Statement.h"
#include "ast/TerminalSymbol.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

// C statement label: id : statement
class LabeledStatement: public Statement {
public:
    LabeledStatement(TerminalSymbol labelName, std::unique_ptr<Statement> statement);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLabel(symbols::AnnotationStore& store, symbols::LabelEntry label);
    symbols::LabelEntry* getLabel(symbols::AnnotationStore& store) const;

    const std::string& getLabelName() const;

    TerminalSymbol name;
    const std::unique_ptr<Statement> statement;

};

} // namespace ast

#endif // LABELEDSTATEMENT_H_
