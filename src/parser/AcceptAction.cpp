#include "AcceptAction.h"

namespace parser {

AcceptAction::AcceptAction() {
}

AcceptAction::~AcceptAction() {
}

bool AcceptAction::parse(std::stack<parse_state>&, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>&) const {
	return true;
}

std::string AcceptAction::serialize() const {
	return "a";
}

} // namespace parser

