#include "SyntaxTreeBuilder.h"

#include "SyntaxTree.h"

using std::string;
using std::vector;
using std::unique_ptr;

SyntaxTreeBuilder::SyntaxTreeBuilder() :
		syntaxTree { new SyntaxTree() },
		currentScope { syntaxTree->getSymbolTable() } {
}

SyntaxTreeBuilder::~SyntaxTreeBuilder() {
}

unique_ptr<SyntaxTree> SyntaxTreeBuilder::build() {
	syntaxTree->setTree(syntaxStack.top());
	return std::unique_ptr<SyntaxTree> { syntaxTree };
}

void SyntaxTreeBuilder::withSourceFileName(std::string fileName) {
	this->sourceFileName = fileName;

	syntaxTree->setFileName(sourceFileName.c_str());
}

vector<Node*> SyntaxTreeBuilder::getChildrenForReduction(int childrenCount) {
	vector<Node*> children;
	for (int i = childrenCount; i > 0; i--) {
		children.push_back(syntaxStack.top());
		syntaxStack.pop();
	}
	return children;
}
