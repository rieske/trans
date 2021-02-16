#ifndef ACCEPTACTION_H_
#define ACCEPTACTION_H_

#include <stack>
#include <string>

#include "Action.h"
#include "TokenStream.h"

namespace parser {

class AcceptAction: public Action {
public:
	AcceptAction();
	virtual ~AcceptAction();

	bool parse(std::stack<parse_state>&, TokenStream&, std::unique_ptr<SyntaxTreeBuilder>&) const override;

	std::string serialize() const override;
};

} // namespace parser

#endif // ACCEPTACTION_H_
