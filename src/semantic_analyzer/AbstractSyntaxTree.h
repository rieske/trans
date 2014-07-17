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
	AbstractSyntaxTree(parser::ParseTreeNode *top, SymbolTable* symbolTable);
	virtual ~AbstractSyntaxTree();

	static const char *getFileName();

	void setFileName(const char *);

	bool hasSemanticErrors() const;

	void outputCode(ostream &of) const;

	SymbolTable *getSymbolTable() const override;
	vector<Quadruple *> getCode() const;

	void printTables() const;

	void outputXml(std::ostream& stream) const override;
	void logTables();
	void logCode();

private:
	parser::ParseTreeNode *tree_deprecated;

	vector<Quadruple *> code;
	SymbolTable *s_table;

	static const char *filename;

	std::unique_ptr<AbstractSyntaxTreeNode> tree;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREE_H_ */
