#include "SyntaxTree.h"

#include <fstream>
#include <sstream>

#include "../code_generator/symbol_table.h"
#include "ParseTreeNode.h"

using std::ofstream;

const char *SyntaxTree::filename = NULL;

SyntaxTree::SyntaxTree(ParseTreeNode *top, SymbolTable* symbolTable) :
		tree { top },
		code { tree->getCode() },
		s_table { symbolTable } {
}

SyntaxTree::~SyntaxTree() {
	delete tree;
	delete s_table;
}

string SyntaxTree::asXml() const {
	ostringstream oss;
	return tree->asXml(oss, 0).str();
}

SymbolTable *SyntaxTree::getSymbolTable() const {
	return s_table;
}

const char *SyntaxTree::getFileName() {
	return filename;
}

void SyntaxTree::setFileName(const char *fname) {
	filename = fname;
}

void SyntaxTree::outputCode(ostream &of) const {
	for (unsigned i = 0; i < code.size(); i++)
		code[i]->output(of);
}

void SyntaxTree::printTables() const {
	s_table->printTable();
}

void SyntaxTree::logTables() {
}

void SyntaxTree::logCode() {
	ofstream outcode;
	outcode.open("logs/int_code");
	if (outcode.is_open()) {
		outputCode(outcode);
		outcode.close();
	} else
		cerr << "Couldn't create file \"logs/int_code\"!\n";
}

vector<Quadruple *> SyntaxTree::getCode() const {
	return code;
}

void SyntaxTree::logXml() {
	string outfile = "logs/syntax_tree.xml";
	std::ofstream xmlfile;
	xmlfile.open(outfile);
	if (!xmlfile.is_open()) {
		std::cerr << "Unable to create syntax tree xml file! Filename: " << outfile << std::endl;
		return;
	}
	xmlfile << asXml();
}
