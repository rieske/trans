#ifndef GENERATEDPARSINGTABLE_H_
#define GENERATEDPARSINGTABLE_H_

#include <string>

#include "parser/CanonicalCollection.h"
#include "parser/ParsingTable.h"

namespace parser {

class GeneratedParsingTable: public ParsingTable {
public:
    GeneratedParsingTable(const Grammar* grammar, AutomatonKind kind = AutomatonKind::LALR1);
    ~GeneratedParsingTable() override = default;

    void persistToFile(std::string fileName) const;

private:
    void computeActionTable(const CanonicalCollection& canonicalCollectionOfSetsOfItems);
    void computeGotoTable(const CanonicalCollection& canonicalCollectionOfSetsOfItems);
    void computeErrorActions(size_t stateCount);
};

} // namespace parser

#endif // GENERATEDPARSINGTABLE_H_
