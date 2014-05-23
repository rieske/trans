#ifndef FIRSTTABLE_H_
#define FIRSTTABLE_H_

#include <iostream>
#include <map>
#include <memory>
#include <vector>

class GrammarSymbol;

class FirstTable {
public:
	FirstTable(const std::vector<std::shared_ptr<const GrammarSymbol>>& symbols);
	virtual ~FirstTable();

	const std::vector<std::shared_ptr<const GrammarSymbol>> operator()(const std::shared_ptr<const GrammarSymbol> symbol) const;

private:
	void initializeTable(const std::vector<std::shared_ptr<const GrammarSymbol>>& symbols);
	bool addFirstSymbol(const std::shared_ptr<const GrammarSymbol>& firstFor, const std::shared_ptr<const GrammarSymbol>& firstSymbol);

	std::map<std::shared_ptr<const GrammarSymbol>, std::vector<std::shared_ptr<const GrammarSymbol>>>firstTable;

	friend std::ostream& operator<<(std::ostream& ostream, const FirstTable& firstTable);
};

#endif /* FIRSTTABLE_H_ */
