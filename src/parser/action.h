#ifndef _ACTION_H_
#define _ACTION_H_

#include <memory>

class GrammarSymbol;
class LR1Item;
class Token;

using std::ofstream;

class Action {
public:
	Action(char t, long s);
	~Action();

	char which() const;
	long getState() const;
	LR1Item getReduction() const;

	void setReduction(LR1Item r);

	void setExpected(std::shared_ptr<const GrammarSymbol> e);
	std::shared_ptr<const GrammarSymbol> getExpected() const;

	void error(const Token& token) const;
	int getForge() const;
	void setForge(unsigned forge);

	void output(ofstream &out) const;

private:
	char type;
	long state;

	std::shared_ptr<LR1Item> reduction;

	std::shared_ptr<const GrammarSymbol> expected;
	int forge_token;
};

#endif // _ACTION_H_
