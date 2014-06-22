#include "ParsingTable.h"

#include <algorithm>

#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "Action.h"
#include "Grammar.h"
#include "LR1Item.h"

using std::string;

static Logger& logger = LogManager::getComponentLogger(Component::PARSER);

ParsingTable::ParsingTable(const Grammar* grammar) :
		grammar { grammar } {
	logger << *this->grammar;
}

ParsingTable::~ParsingTable() {
}

Action& ParsingTable::action(parse_state state, string lookahead) const {
	return *lookaheadActions.at(state).at(lookahead);
}

parse_state ParsingTable::go_to(parse_state state, const GrammarSymbol* nonterminal) const {
	return gotoTable.at(state).at(nonterminal);
}

LR1Item ParsingTable::getReductionById(std::string nonterminalId, size_t productionId) const {
	const auto& nonterminals = grammar->getNonterminals();
	const auto& nonterminalIterator = std::find_if(nonterminals.begin(), nonterminals.end(),
			[&nonterminalId] (const GrammarSymbol* nonterminal) {return nonterminal->getSymbol() == nonterminalId;});
	if (nonterminalIterator == nonterminals.end()) {
		throw std::invalid_argument("Nonterminal not found by id " + nonterminalId);
	}
	return {*nonterminalIterator, productionId, {}};
}
