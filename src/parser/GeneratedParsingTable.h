#ifndef GENERATEDPARSINGTABLE_H_
#define GENERATEDPARSINGTABLE_H_

#include <string>
#include <vector>

#include "parser/FirstTable.h"
#include "parser/ParsingTable.h"
#include "parser/CanonicalCollection.h"

namespace parser {

class GeneratedParsingTable: public ParsingTable {
public:
    GeneratedParsingTable(const Grammar* grammar, const CanonicalCollectionStrategy& canonicalCollectionStrategy);
    virtual ~GeneratedParsingTable();

    void persistToFile(std::string fileName) const;

private:
    void computeActionTable(const CanonicalCollection& canonicalCollectionOfSetsOfItems);
    void computeGotoTable(const CanonicalCollection& canonicalCollectionOfSetsOfItems);
    void computeErrorActions(size_t stateCount);

    FirstTable firstTable;
};

} // namespace parser

#endif // GENERATEDPARSINGTABLE_H_
