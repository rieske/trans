#ifndef PRODUCTION_H_
#define PRODUCTION_H_

#include <initializer_list>
#include <string>
#include <vector>

namespace parser {

class GrammarSymbol;

class Production {
private:
    std::vector<GrammarSymbol> symbolSequence;
    std::size_t id;

public:
    Production(std::initializer_list<GrammarSymbol> symbolSequence, std::size_t id);
    Production(std::vector<GrammarSymbol> symbolSequence, std::size_t id);
    virtual ~Production();

    const auto begin() const -> decltype(symbolSequence.begin());
    const auto end() const -> decltype(symbolSequence.end());
    auto size() const -> decltype(symbolSequence.size());

    bool produces(const std::vector<std::string>& sequence) const;

    std::vector<std::string> producedSequence() const;

    std::size_t getId() const;
};

} /* namespace parser */

#endif /* PRODUCTION_H_ */
