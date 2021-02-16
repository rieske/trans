#ifndef FINITEAUTOMATON_H_
#define FINITEAUTOMATON_H_

#include "scanner/State.h"
#include <map>
#include <string>

class FiniteAutomaton {
public:
    FiniteAutomaton(State* startState, std::map<std::string, unsigned> keywordIds);
    virtual ~FiniteAutomaton();

    void updateState(char inputSymbol);

    bool isAtFinalState() const;
    std::string getAccumulatedLexeme() const;
    std::string getAccumulatedToken() const;

protected:
    FiniteAutomaton() {
    }

    const State* startState { nullptr };
    const State* currentState { nullptr };
    std::map<std::string, unsigned> keywordIds;
private:

    std::string accumulator;
    std::string accumulatedLexeme;
    std::string accumulatedToken;
};

#endif // FINITEAUTOMATON_H_
