#include "AbstractSyntaxTree.h"

#include <stddef.h>
#include <fstream>

#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeNode.h"
#include "XmlOutputVisitor.h"

namespace semantic_analyzer {

const char *AbstractSyntaxTree::filename = NULL;

AbstractSyntaxTree::AbstractSyntaxTree(AbstractSyntaxTreeNode *top, SymbolTable* symbolTable) :
		tree { top },
		s_table { symbolTable } {
	code = this->tree->getCode();
}

AbstractSyntaxTree::~AbstractSyntaxTree() {
	delete s_table;
}

SymbolTable *AbstractSyntaxTree::getSymbolTable() const {
	return s_table;
}

const char *AbstractSyntaxTree::getFileName() {
	return filename;
}

void AbstractSyntaxTree::setFileName(const char *fname) {
	filename = fname;
}

void AbstractSyntaxTree::outputCode(ostream &of) const {
	for (unsigned i = 0; i < code.size(); i++)
		code[i]->output(of);
}

void AbstractSyntaxTree::printTables() const {
	s_table->printTable();
}

void AbstractSyntaxTree::logTables() {
}

void AbstractSyntaxTree::logCode() {
	std::ofstream outcode;
	outcode.open("logs/int_code");
	if (outcode.is_open()) {
		outputCode(outcode);
		outcode.close();
	} else
		std::cerr << "Couldn't create file \"logs/int_code\"!\n";
}

vector<Quadruple *> AbstractSyntaxTree::getCode() const {
	return code;
}

void AbstractSyntaxTree::outputXml(std::ostream& stream) const {
	XmlOutputVisitor xmlOutputer { &stream };
	tree->accept(xmlOutputer);
}

} /* namespace semantic_analyzer */
