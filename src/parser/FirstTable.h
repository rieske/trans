#ifndef FIRSTTABLE_H_
#define FIRSTTABLE_H_

#include <map>
#include <memory>
#include <vector>

class GrammarRule;
class GrammarSymbol;

class FirstTable {
public:
	FirstTable(const std::vector<std::shared_ptr<GrammarRule>>& grammarRules);
	virtual ~FirstTable();

	const std::vector<std::shared_ptr<GrammarSymbol>> firstSet(const std::shared_ptr<GrammarSymbol> symbol);

private:
	bool addFirst(std::shared_ptr<GrammarSymbol> symbol, std::shared_ptr<GrammarSymbol> first);
	bool addFirstRow(std::shared_ptr<GrammarSymbol> dest, std::shared_ptr<GrammarSymbol> src);

	std::map<std::shared_ptr<GrammarSymbol>, std::vector<std::shared_ptr<GrammarSymbol>>>firstTable;
};

#endif /* FIRSTTABLE_H_ */
