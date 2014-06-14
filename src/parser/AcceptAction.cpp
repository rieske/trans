#include "AcceptAction.h"

#include "../semantic_analyzer/SyntaxTree.h"

using std::stack;
using std::unique_ptr;
using std::string;

AcceptAction::AcceptAction() {
}

AcceptAction::~AcceptAction() {
}

unique_ptr<SyntaxTree> AcceptAction::perform(stack<parse_state>&, TokenStream&, SemanticAnalyzer& semanticAnalyzer) const {
	return semanticAnalyzer.build();
}

string AcceptAction::describe() const {
	return "a 0";
}
