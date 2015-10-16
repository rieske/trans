#include "parser/AcceptAction.h"

using std::stack;
using std::string;

namespace parser {

AcceptAction::AcceptAction() {
}

AcceptAction::~AcceptAction() {
}

bool AcceptAction::parse(stack<parse_state>&, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>&) const {
	return true;
}

string AcceptAction::serialize() const {
	return "a";
}

}
