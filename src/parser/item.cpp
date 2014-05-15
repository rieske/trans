#include "item.h"

#include <bits/shared_ptr_base.h>
#include <iostream>
#include <iterator>

#include "GrammarSymbol.h"

using std::vector;

Item::Item(std::shared_ptr<GrammarSymbol> str) {
	left = str;
}

Item::~Item() {
}

bool Item::operator==(const Item& rhs) const {
	if (this->left != rhs.left)
		return false;
	if (this->seen != rhs.seen)
		return false;
	if (this->expected != rhs.expected)
		return false;
	if (this->lookaheads != rhs.lookaheads)
		return false;
	return true;
}

bool Item::operator!=(const Item& rhs) const {
	return !(*this == rhs);
}

void Item::print() const {
	log(std::cerr);
}

void Item::log(std::ostream &out) const {
	out << "[ " << left << " -> ";
	for (unsigned i = 0; i < seen.size(); i++)
		out << seen.at(i) << " ";
	out << ". ";
	for (auto& expectedSymbol : expected) {
		out << *expectedSymbol << " ";
	}
	out << ", ";
	for (unsigned i = 0; i < lookaheads.size(); i++) {
		out << lookaheads.at(i);
		if (i != lookaheads.size() - 1)
			out << "/";
	}
	out << " ]\n";
}

bool Item::coresAreEqual(Item& rhs) const {
	if (this->left != rhs.left)
		return false;
	if (this->seen != rhs.seen)
		return false;
	if (this->expected != rhs.expected)
		return false;
	return true;
}

void Item::mergeLookaheads(const Item& it) {
	if (lookaheads.empty()) {
		for (unsigned i = 0; i < it.lookaheads.size(); i++)
			lookaheads.push_back(it.lookaheads.at(i));
		return;
	}
	for (unsigned i = 0; i < it.lookaheads.size(); i++) {
		bool insert = true;
		unsigned j = 0;
		for (; j < this->lookaheads.size(); j++) {
			if (it.lookaheads.at(i) == this->lookaheads.at(j)) {
				insert = false;
				break;
			}
		}
		if (insert)
			this->lookaheads.push_back(it.lookaheads.at(i));
	}
}

void Item::addSeen(std::shared_ptr<GrammarSymbol> symbol) {
	seen.push_back(symbol);
}

void Item::addExpected(std::shared_ptr<GrammarSymbol> expectedSymbols) {
	expected.push_back(expectedSymbols);
}

void Item::setExpected(vector<std::shared_ptr<GrammarSymbol>> expected) {
	this->expected = expected;
}

void Item::addLookahead(std::shared_ptr<GrammarSymbol> lookahead) {
	lookaheads.push_back(lookahead);
}

std::shared_ptr<GrammarSymbol> Item::getLeft() const {
	return left;
}

vector<std::shared_ptr<GrammarSymbol>> Item::getSeen() const {
	return seen;
}

vector<std::shared_ptr<GrammarSymbol>> Item::getExpected() const {
	return expected;
}

vector<std::shared_ptr<GrammarSymbol>> Item::getLookaheads() const {
	return lookaheads;
}

