#ifndef BNFREADER_H_
#define BNFREADER_H_

#include <memory>
#include <string>
#include <vector>

#include "Grammar.h"
#include "GrammarSymbol.h"
#include "Production.h"

namespace parser {

class BNFFileGrammar: public Grammar {
public:
	BNFFileGrammar(const std::string bnfFileName);
	virtual ~BNFFileGrammar();

	std::size_t ruleCount() const override;
	const Production& getRuleByIndex(int index) const override;
	std::vector<Production> getProductionsOfSymbol(const GrammarSymbol* symbol) const override;

	std::vector<const GrammarSymbol*> getTerminals() const override;
	std::vector<const GrammarSymbol*> getNonterminals() const override;

private:
	GrammarSymbol* addSymbol(const std::string& name);

	std::vector<std::unique_ptr<GrammarSymbol>> symbols;

	std::vector<Production> rules;

	std::vector<const GrammarSymbol*> terminals;
	std::vector<const GrammarSymbol*> nonterminals;
};

}

#endif /* BNFREADER_H_ */
