#ifndef BNFREADER_H_
#define BNFREADER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

class GrammarRule;
class GrammarSymbol;
class NonterminalSymbol;

class BNFReader {
public:
	BNFReader(const std::string bnfFileName);
	virtual ~BNFReader();

	std::vector<std::shared_ptr<GrammarSymbol>> getTerminals() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getNonterminals() const;

	std::map<int, std::shared_ptr<GrammarSymbol>> getIdToTerminalMappingTable() const;

private:
	std::shared_ptr<GrammarSymbol> findTerminalByName(const std::string& name) const;
	std::shared_ptr<GrammarSymbol> addTerminal(const std::string& name);
	std::shared_ptr<NonterminalSymbol> addUndefinedNonterminal(const std::string& name,
			std::vector<std::shared_ptr<NonterminalSymbol>>& undefinedNonterminals);

	std::vector<std::shared_ptr<GrammarSymbol>> terminals;
	std::vector<std::shared_ptr<GrammarSymbol>> nonterminals;

	size_t nextSymbolId { 1 };

	// FIXME: get rid of me
	std::map<int, std::shared_ptr<GrammarSymbol>> idToTerminalMappingTable;
};

#endif /* BNFREADER_H_ */
