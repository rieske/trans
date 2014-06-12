#include "action.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../scanner/Token.h"
#include "GrammarSymbol.h"
#include "LR1Item.h"

using std::cerr;
using std::endl;
using std::shared_ptr;
using std::string;

Action::Action(char t, long s) {
	type = t;
	state = s;
}

Action::~Action() {
}

char Action::which() const {
	return type;
}

long Action::getState() const {
	return state;
}

LR1Item Action::getReduction() const {
	return *reduction;
}

void Action::setReduction(LR1Item r) {
	reduction = std::make_shared<LR1Item>(r);
}

/**
 *  surašo duomenis apie save į failą vieno stringo pavidalu
 *  formatas: type State [forge_token Expected]|[nonterminalId reduction_id]
 **/
void Action::output(ofstream &out) const {
	out << type << " " << state;
	switch (type) {
	case 'r':
		out << " " << reduction->getDefiningSymbol()->getId() << " " << reduction->getProductionId();
		break;
	case 'e':
		out << " " << (forge_token.empty() ? "NOFORGE" : forge_token) << " " << *expected;
		break;
	default:
		break;
	}
}

void Action::error(const Token& token) const {
	if (token.lexeme.empty()) {
		throw std::runtime_error("Error at end of input file! ");
	}
	cerr << "Error on line " << token.line << ": " << *expected << " expected, got: " << token.lexeme << endl;
}

void Action::setExpected(std::shared_ptr<const GrammarSymbol> e) {
	expected = e;
}

std::shared_ptr<const GrammarSymbol> Action::getExpected() const {
	return expected;
}

string Action::getForge() const {
	return forge_token;
}

void Action::setForge(string forge) {
	forge_token = forge;
}
