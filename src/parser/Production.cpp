#include "Production.h"

#include <stdexcept>

#include "GrammarSymbol.h"

namespace parser {

Production::Production(std::initializer_list<const GrammarSymbol*> symbolSequence) :
		Production(std::vector<const GrammarSymbol*> { symbolSequence }) {
}

Production::Production(std::vector<const GrammarSymbol*> symbolSequence) :
		symbolSequence { symbolSequence } {
	if (this->symbolSequence.empty()) {
		throw std::invalid_argument { "can not produce empty symbol sequence" };
	}
}

Production::~Production() {
}

const auto Production::begin() const -> decltype(symbolSequence.begin()) {
	return symbolSequence.begin();
}

const auto Production::end() const -> decltype(symbolSequence.end()) {
	return symbolSequence.end();
}

auto Production::size() const -> decltype(symbolSequence.size()) {
	return symbolSequence.size();
}

bool Production::produces(std::vector<std::string> sequence) const{
	if (sequence.size() != symbolSequence.size()) {
		return false;
	}
	for (size_t i = 0; i != sequence.size(); ++i) {
		if (sequence.at(i) != symbolSequence.at(i)->getDefinition()) {
			return false;
		}
	}
	return true;
}

std::vector<std::string> Production::producedSequence() const {
	std::vector<std::string> producedSequence;
	for (const auto& symbol : symbolSequence) {
		producedSequence.push_back(symbol->getDefinition());
	}
	return producedSequence;
}

} /* namespace parser */
