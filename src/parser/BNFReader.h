#ifndef BNFREADER_H_
#define BNFREADER_H_

#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class GrammarSymbol;

class BNFReader {
public:
	BNFReader(const std::string bnfFileName);
	virtual ~BNFReader();

	std::vector<std::shared_ptr<const GrammarSymbol>> getTerminals() const;
	std::vector<std::shared_ptr<const GrammarSymbol>> getNonterminals() const;

	std::map<int, std::shared_ptr<const GrammarSymbol>> getIdToTerminalMappingTable() const;

private:
	std::shared_ptr<const GrammarSymbol> findTerminalByName(const std::string& name) const;
	std::shared_ptr<const GrammarSymbol> addTerminal(const std::string& name);
	std::shared_ptr<GrammarSymbol> addUndefinedNonterminal(const std::string& name,
			std::vector<std::shared_ptr<GrammarSymbol>>& undefinedNonterminals);

	std::vector<std::shared_ptr<const GrammarSymbol>> terminals;
	std::vector<std::shared_ptr<const GrammarSymbol>> nonterminals;

	size_t nextSymbolId { 1 };

	// FIXME: get rid of me
	std::map<int, std::shared_ptr<const GrammarSymbol>> idToTerminalMappingTable;
};

#endif /* BNFREADER_H_ */
