#ifndef _STATE_H_
#define _STATE_H_

#include <iostream>
#include <map>
#include <string>

namespace scanner {

class State {
public:
    State(std::string stateName, std::string tokenId);
    virtual ~State();

    void addTransition(std::string charactersForTransition, State* state);
    virtual const State* nextStateForCharacter(char c) const;
    std::string getName() const;

    void outputState(std::ostream& ostream) const;

    std::string getTokenId() const;
    virtual bool needsKeywordLookup() const;
    bool isFinal() const;

private:
    std::string stateName;
    std::string tokenId;

    State* wildcardTransition;
    std::map<char, State*> transitions;
};

class IdentifierState: public State {
public:
    IdentifierState(std::string stateName, std::string tokenId);
    virtual ~IdentifierState();

    bool needsKeywordLookup() const override;
};

class StringLiteralState: public State {
public:
    StringLiteralState(std::string stateName, std::string tokenId);
    virtual ~StringLiteralState();

    const State* nextStateForCharacter(char c) const override;
};

class EOLCommentState: public State {
public:
    EOLCommentState(std::string stateName);
    virtual ~EOLCommentState();

    const State* nextStateForCharacter(char c) const override;
};

} // namespace scanner

#endif // _STATE_H_
