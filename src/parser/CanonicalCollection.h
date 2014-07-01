#ifndef CANONICALCOLLECTION_H_
#define CANONICALCOLLECTION_H_

#include <stddef.h>
#include <vector>

#include "FirstTable.h"
#include "LR1Item.h"

class Grammar;

class CanonicalCollection {
public:
	CanonicalCollection(const FirstTable& firstTable, const Grammar& grammar);
	virtual ~CanonicalCollection();

	size_t stateCount() const noexcept;
	std::vector<LR1Item> setOfItemsAtState(size_t state) const;
	bool contains(const std::vector<LR1Item>& setOfItems) const;
	size_t stateFor(const std::vector<LR1Item>& setOfItems) const;

private:
	void logCollection() const;

	const FirstTable firstTable;

	std::vector<std::vector<LR1Item>> canonicalCollection;
};

#endif /* CANONICALCOLLECTION_H_ */
