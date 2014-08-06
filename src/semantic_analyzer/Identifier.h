#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_

#include "Declaration.h"

namespace semantic_analyzer {

class TerminalSymbol;

class Identifier: public Declaration {
public:
	Identifier(TerminalSymbol identifier);
	virtual ~Identifier();

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;
};

} /* namespace semantic_analyzer */

#endif /* IDENTIFIER_H_ */
