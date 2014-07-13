#include "AcceptAction.h"

using std::stack;
using std::string;

namespace parser {

AcceptAction::AcceptAction() {
}

AcceptAction::~AcceptAction() {
}

bool AcceptAction::parse(stack<parse_state>&, TokenStream&, SemanticAnalyzer&) const {
	return true;
}

string AcceptAction::serialize() const {
	return "a";
}

}
