#ifndef _RULE_H_
#define _RULE_H_

#include <memory>
#include <vector>

class GrammarSymbol;

using std::string;
using std::vector;
using std::ostream;

/**
 * @author Vaidotas Valuckas
 * Gramatikos taisyklės klasė
 * [ left -> right ]
 **/

class GrammarRule {
public:
	GrammarRule(std::shared_ptr<GrammarSymbol> l, int ruleId);
	~GrammarRule();

	std::shared_ptr<GrammarSymbol> getLeft() const;
	vector<std::shared_ptr<GrammarSymbol>> *getRight() const;

	string rightStr() const;

	unsigned getId() const;

	void addRight(std::shared_ptr<GrammarSymbol> r);

	void print() const;
	void printAddr() const;

	void log(ostream &out) const;

private:
	unsigned id;
	std::shared_ptr<GrammarSymbol> left;
	vector<std::shared_ptr<GrammarSymbol>> *right;
};

#endif // _RULE_H_
