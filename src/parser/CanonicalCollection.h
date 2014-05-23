#ifndef CANONICALCOLLECTION_H_
#define CANONICALCOLLECTION_H_

#include <vector>

#include "FirstTable.h"
#include "LR1Item.h"

class Grammar;

class CanonicalCollection {
public:
	CanonicalCollection(const FirstTable& firstTable);
	virtual ~CanonicalCollection();

	std::vector<std::vector<LR1Item>> computeForGrammar(const Grammar& grammar) const;

private:
	const FirstTable firstTable;
};

#endif /* CANONICALCOLLECTION_H_ */
