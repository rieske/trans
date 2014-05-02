#include "SyntaxTreeBuilder.h"

using std::string;

SyntaxTreeBuilder::~SyntaxTreeBuilder() {
}

void SyntaxTreeBuilder::withSourceFileName(std::string fileName) {
	this->sourceFileName = fileName;
}
