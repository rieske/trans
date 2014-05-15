#ifndef FIRSTTABLE_H_
#define FIRSTTABLE_H_

#include <iostream>
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
	void initializeTable(const std::vector<std::shared_ptr<GrammarRule>>& grammarRules);
	bool addFirstSymbol(const std::shared_ptr<GrammarSymbol>& firstFor, const std::shared_ptr<GrammarSymbol>& firstSymbol);

	std::map<std::shared_ptr<GrammarSymbol>, std::vector<std::shared_ptr<GrammarSymbol>>>firstTable;

	friend std::ostream& operator<<(std::ostream& ostream, const FirstTable& firstTable);
};

#endif /* FIRSTTABLE_H_ */
