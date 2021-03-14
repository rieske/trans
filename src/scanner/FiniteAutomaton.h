#ifndef FINITEAUTOMATON_H_
#define FINITEAUTOMATON_H_

#include "scanner/State.h"
#include <map>
#include <string>
#include <memory>

namespace scanner {

class FiniteAutomaton {
public:
    FiniteAutomaton(
            State* startState,
            std::map<std::string, int> keywordIds,
            std::map<std::string, std::unique_ptr<State>> namedStates);

    void updateState(char inputSymbol);

    bool isAtFinalState() const;
    std::string getAccumulatedLexeme() const;
    std::string getAccumulatedToken() const;

private:
    const State* startState { nullptr };
    const State* currentState { nullptr };
    std::map<std::string, int> keywordIds;
    std::map<std::string, std::unique_ptr<State>> namedStates;

    std::string accumulator;
    std::string accumulatedLexeme;
    std::string accumulatedToken;
};

} // namespace scanner

#endif // FINITEAUTOMATON_H_
