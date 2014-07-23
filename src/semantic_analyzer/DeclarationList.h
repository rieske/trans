#ifndef _DECLS_NODE_H_
#define _DECLS_NODE_H_

#include <string>
#include <vector>

#include "Declaration.h"

namespace semantic_analyzer {

class DeclarationList: public NonterminalNode {
public:
	DeclarationList(Declaration* declaration);

	void addDeclaration(Declaration* declaration);
	vector<Declaration*> getDeclarations() const;

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

private:
	vector<Declaration*> declarations;
};

}

#endif // _DECLS_NODE_H_
