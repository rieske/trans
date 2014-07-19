#ifndef NONTERMINALNODEFACTORY_H_
#define NONTERMINALNODEFACTORY_H_

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

namespace semantic_analyzer {

class NonterminalNode;

class NonterminalNodeFactory {
public:
	NonterminalNodeFactory();
	virtual ~NonterminalNodeFactory();

	std::unique_ptr<NonterminalNode> makeNonterminalNode(std::string definingSymbol);

private:
	std::unordered_map<std::string, std::function<std::unique_ptr<NonterminalNode>(void)>> nodeRegistry;
};

} /* namespace semantic_analyzer */

#endif /* NONTERMINALNODEFACTORY_H_ */
