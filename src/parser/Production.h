#ifndef PRODUCTION_H_
#define PRODUCTION_H_

#include <initializer_list>
#include <string>
#include <vector>

namespace parser {

class GrammarSymbol;

class Production {
private:
	const std::vector<const GrammarSymbol*> symbolSequence;

public:
	Production(std::initializer_list<const GrammarSymbol*> symbolSequence);
	Production(std::vector<const GrammarSymbol*> symbolSequence);
	virtual ~Production();

	const auto begin() const -> decltype(symbolSequence.begin());
	const auto end() const -> decltype(symbolSequence.end());
	auto size() const -> decltype(symbolSequence.size());

	bool produces(std::vector<std::string> sequence) const;

	std::vector<std::string> producedSequence() const;
};

} /* namespace parser */

#endif /* PRODUCTION_H_ */
