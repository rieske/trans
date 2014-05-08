#ifndef _ACTION_H_
#define _ACTION_H_

#include <memory>

#include "GrammarRule.h"

class GrammarSymbol;
class GrammarRule;
class Token;

using std::ofstream;

class Action {
public:
	Action(char t, long s);
	~Action();

	char which() const;
	long getState() const;
	std::shared_ptr<GrammarRule> getReduction() const;

	void setReduction(std::shared_ptr<GrammarRule> r);

	void setExpected(std::shared_ptr<GrammarSymbol> e);
	std::shared_ptr<GrammarSymbol> getExpected() const;

	void error(Token *) const;
	unsigned getForge() const;
	void setForge(unsigned forge);

	void print() const;
	void log(ofstream &out) const;
	void output(ofstream &out) const;

private:
	char type;
	long state;

	std::shared_ptr<GrammarRule> reduction;

	std::shared_ptr<GrammarSymbol> expected;
	unsigned forge_token;
};

#endif // _ACTION_H_
