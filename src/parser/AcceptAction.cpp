#include "AcceptAction.h"

#include "SyntaxTree.h"

using std::stack;
using std::unique_ptr;
using std::string;

AcceptAction::AcceptAction() {
}

AcceptAction::~AcceptAction() {
}

unique_ptr<SyntaxTree> AcceptAction::perform(stack<parse_state>&, TokenStream&, SemanticAnalyzer& semanticAnalyzer) const {
	return semanticAnalyzer.getSyntaxTree();
}

string AcceptAction::serialize() const {
	return "a";
}
