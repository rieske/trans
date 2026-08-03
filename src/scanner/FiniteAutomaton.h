#ifndef FINITEAUTOMATON_H_
#define FINITEAUTOMATON_H_

#include "scanner/State.h"
#include <map>
#include <string>
#include <memory>

namespace scanner {

class TypedefRegistry;

class FiniteAutomaton {
public:
    FiniteAutomaton(
            State* startState,
            std::map<std::string, int> keywordIds,
            std::map<std::string, std::unique_ptr<State>> namedStates);

    void updateState(char inputSymbol);

    bool isAtStartState() const;
    bool isAtFinalState() const;
    std::string getAccumulatedLexeme() const;
    std::string getAccumulatedToken() const;

    // Lexer feedback: typedef names live only on the session TypedefRegistry.
    void setTypedefRegistry(TypedefRegistry* registry) { typedefs_ = registry; }
    TypedefRegistry* typedefRegistry() const { return typedefs_; }

private:
    bool isTypedefName(const std::string& name) const;

    const State* startState { nullptr };
    const State* currentState { nullptr };
    std::map<std::string, int> keywordIds;
    std::map<std::string, std::unique_ptr<State>> namedStates;

    std::string accumulator;
    std::string accumulatedLexeme;
    std::string accumulatedToken;

    TypedefRegistry* typedefs_ { nullptr };
};

} // namespace scanner

#endif // FINITEAUTOMATON_H_
