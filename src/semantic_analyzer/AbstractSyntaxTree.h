#ifndef ABSTRACTSYNTAXTREE_H_
#define ABSTRACTSYNTAXTREE_H_

#include <iostream>
#include <memory>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../parser/SyntaxTree.h"

namespace semantic_analyzer {

class AbstractSyntaxTreeNode;

// FIXME:
class AbstractSyntaxTree: public parser::SyntaxTree {
public:
    AbstractSyntaxTree(std::unique_ptr<AbstractSyntaxTreeNode> top, SymbolTable* symbolTable);
    virtual ~AbstractSyntaxTree();

    void analyzeWith(semantic_analyzer::SemanticAnalyzer& semanticAnalyzer);

    void outputCode(ostream &of) const;

    SymbolTable *getSymbolTable() const;
    vector<Quadruple *> getCode() const;

    void printTables() const;
    void logCode();

    void outputXml(std::ostream& stream) const override;
    void outputSource(std::ostream& stream) const override;

private:
    vector<Quadruple *> code;
    SymbolTable *s_table;

    std::unique_ptr<AbstractSyntaxTreeNode> tree;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREE_H_ */
