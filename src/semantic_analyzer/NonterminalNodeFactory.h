#ifndef NONTERMINALNODEFACTORY_H_
#define NONTERMINALNODEFACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>

namespace parser {
class NonterminalNode;
} /* namespace parser */

namespace semantic_analyzer {

class NonterminalNodeFactory {
public:
	NonterminalNodeFactory();
	virtual ~NonterminalNodeFactory();

	std::unique_ptr<parser::NonterminalNode> makeNonterminalNode(std::string definingSymbol);

private:
	std::unordered_map<std::string, std::function<std::unique_ptr<parser::NonterminalNode>(void)>> nodeRegistry;
};

} /* namespace semantic_analyzer */

#endif /* NONTERMINALNODEFACTORY_H_ */
