#ifndef BNFREADER_H_
#define BNFREADER_H_

#include <stddef.h>
#include <memory>
#include <string>
#include <vector>

#include "Grammar.h"

class GrammarSymbol;

class BNFFileGrammar: public Grammar {
public:
	BNFFileGrammar(const std::string bnfFileName);
	virtual ~BNFFileGrammar();

	std::vector<const GrammarSymbol*> getTerminals() const override;
	std::vector<const GrammarSymbol*> getNonterminals() const override;

private:
	GrammarSymbol* addSymbol(const std::string& name);

	size_t nextSymbolId { 1 };

	std::vector<std::unique_ptr<GrammarSymbol>> symbols;

	std::vector<const GrammarSymbol*> terminals;
	std::vector<const GrammarSymbol*> nonterminals;
};

#endif /* BNFREADER_H_ */
