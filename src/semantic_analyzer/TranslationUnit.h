#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <memory>
#include <vector>

#include "NonterminalNode.h"
#include "ListCarrier.h"

namespace semantic_analyzer {

class TranslationUnit: public NonterminalNode {
public:
	TranslationUnit(std::unique_ptr<ListCarrier> functionDeclarations);
	TranslationUnit(std::unique_ptr<ListCarrier> variableDeclarations, std::unique_ptr<ListCarrier> functionDeclarations);
	virtual ~TranslationUnit();

	const std::vector<std::unique_ptr<ListCarrier>>& getChildren() const;

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

private:
	std::vector<std::unique_ptr<ListCarrier>> children;
};

} /* namespace semantic_analyzer */

#endif /* TRANSLATIONUNIT_H_ */
