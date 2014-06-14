#ifndef GENERATEDPARSINGTABLE_H_
#define GENERATEDPARSINGTABLE_H_

#include <memory>
#include <vector>

#include "LR1Item.h"
#include "ParsingTable.h"

class FirstTable;
class GoTo;
class Grammar;

class GeneratedParsingTable: public ParsingTable {
public:
	GeneratedParsingTable(const Grammar& grammar);
	virtual ~GeneratedParsingTable();

	void output_table(const Grammar& grammar) const;
private:
	int fill_actions(std::vector<std::vector<LR1Item>> C, const Grammar& grammar);
	int fill_goto(std::vector<std::vector<LR1Item>> C, const Grammar& grammar);
	void fill_errors(const Grammar& grammar);

	void logCanonicalCollection(std::vector<std::vector<LR1Item>>& items) const;

	std::unique_ptr<FirstTable> firstTable;
	std::unique_ptr<GoTo> goTo;

	int state_count;

	std::vector<std::vector<LR1Item>> items;
};

#endif /* GENERATEDPARSINGTABLE_H_ */
