#include "item.h"

#include <bits/shared_ptr_base.h>
#include <iostream>
#include <iterator>

#include "GrammarSymbol.h"

using std::cerr;
using std::endl;

Item::Item(std::shared_ptr<GrammarSymbol> str) {
	left = str;
	local_expected = true;
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
	if (this->lookaheads.size() != rhs.lookaheads.size())
		return false;
	if (!compare_lookaheads(rhs))
		return false;
	return true;
}

bool Item::compare_lookaheads(const Item& rhs) const {
	unsigned from = 0;
	for (vector<std::shared_ptr<GrammarSymbol>>::const_iterator it1 = lookaheads.begin(); it1 != lookaheads.end(); it1++) {
		bool contains = false;
		for (vector<std::shared_ptr<GrammarSymbol>>::const_iterator it2 = rhs.lookaheads.begin() + from; it2 != rhs.lookaheads.end();
				it2++) {
			if (*it1 == *it2) {
				contains = true;
				from++;
				break;
			}
		}
		if (!contains)
			return false;
	}
	return true;
}

bool Item::operator!=(const Item& rhs) const {
	return !(*this == rhs);
}

void Item::print() const {
	cerr << "[ " << *left << " -> ";
	for (unsigned i = 0; i < seen.size(); i++)
		cerr << *seen.at(i) << " ";
	cerr << ". ";
	for (auto& expectedSymbol : expected) {
		cerr << *expectedSymbol << " ";
	}
	cerr << ", ";
	for (unsigned i = 0; i < lookaheads.size(); i++) {
		cerr << *lookaheads.at(i);
		if (i != lookaheads.size() - 1)
			cerr << "/";
	}
	cerr << " ]" << endl;
}

void Item::log(ostream &out) const {
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
	out << " ]" << endl;
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
	local_expected = false;
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

