#ifndef IDENTIFIER_H_
#define IDENTIFIER_H_

#include "DirectDeclaration.h"

namespace ast {

class TerminalSymbol;

class Identifier: public DirectDeclaration {
public:
	Identifier(TerminalSymbol identifier);
	virtual ~Identifier();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} /* namespace ast */

#endif /* IDENTIFIER_H_ */
