#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_

#include "Declaration.h"

namespace ast {

class TerminalSymbol;

class Identifier: public Declaration {
public:
	Identifier(TerminalSymbol identifier);
	virtual ~Identifier();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} /* namespace ast */

#endif /* IDENTIFIER_H_ */
