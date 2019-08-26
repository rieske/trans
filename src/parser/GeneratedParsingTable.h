#ifndef GENERATEDPARSINGTABLE_H_
#define GENERATEDPARSINGTABLE_H_

#include <stddef.h>
#include <string>
#include <vector>

#include "FirstTable.h"
#include "LR1Item.h"
#include "ParsingTable.h"

namespace parser {

class CanonicalCollectionStrategy;
class CanonicalCollection;
class Grammar;

class GeneratedParsingTable: public ParsingTable {
public:
    GeneratedParsingTable(const Grammar* grammar, const CanonicalCollectionStrategy& canonicalCollectionStrategy);
    virtual ~GeneratedParsingTable();

    void outputPretty(std::string fileName) const;

    void persistToFile(std::string fileName) const;

private:
    void computeActionTable(const CanonicalCollection& canonicalCollectionOfSetsOfItems);
    void computeGotoTable(const CanonicalCollection& canonicalCollectionOfSetsOfItems);
    void computeErrorActions(size_t stateCount);

    FirstTable firstTable;
};

}

#endif /* GENERATEDPARSINGTABLE_H_ */
