#ifndef FIRSTTABLE_H_
#define FIRSTTABLE_H_

#include <iostream>
#include <map>
#include <vector>

#include "GrammarSymbol.h"

class FirstTable {
public:
	FirstTable(const std::vector<const GrammarSymbol*>& symbols);
	virtual ~FirstTable();

	const std::vector<const GrammarSymbol*> operator()(const GrammarSymbol* symbol) const;

private:
	void initializeTable(const std::vector<const GrammarSymbol*>& symbols);
	bool addFirstSymbol(const GrammarSymbol* firstFor, const GrammarSymbol* firstSymbol);

	std::map<const GrammarSymbol*, std::vector<const GrammarSymbol*>> firstTable;

	friend std::ostream& operator<<(std::ostream& ostream, const FirstTable& firstTable);
};

#endif /* FIRSTTABLE_H_ */
