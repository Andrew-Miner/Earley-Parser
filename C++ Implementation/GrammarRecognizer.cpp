#include "GrammarRecognizer.h"
#include "Terminal.h"
#include "NonTerminal.h"
#include <typeinfo>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace egp;

bool egp::compareStart(const EarlyItem& first, const EarlyItem& second)
{
	return first.start > second.start;
}

EarlyVec egp::buildItems(const Grammar& g, const std::string& input)
{
	std::unordered_set<std::string> nullableRules = getNullableRules(g);
	EarlyVec s = { {} };
	
	// initialize s[0] set
	for (int i = 0; i < g.rules.size(); i++) {
		if (g.rules[i].name == g.startRule) {
			s[0].push_back({ i, 0, 0 }); // EarlyItem: {rule, next, start}
		}
	}

	// populate the rest of s[i]
	int sSize = s.size();
	for (int i = 0; i < sSize; i++) {
		int setSize = s[i].size();
		for (int j = 0; j < setSize; j++) {
			Symbol* symbol = nextSymbol(g, s[i][j]);
			if (symbol == nullptr)
				complete(s, i, j, setSize, g);
			else if (typeid(*symbol) == typeid(Terminal))
				scan(s, i, j, sSize, symbol, input);
			else if (typeid(*symbol) == typeid(NonTerminal))
				predict(s, i, j, setSize, symbol, g, nullableRules);
			else
				throw "illegal rule";
		}
	}
	return s;
}


Symbol* egp::nextSymbol(const Grammar& g, const EarlyItem& item)
{
	if (g.rules[item.rule].definition.size() <= item.next)
		return nullptr;
	return g.rules[item.rule].definition[item.next];
}

void egp::complete(EarlyVec& s, int i, int j, int& size, const Grammar& g)
{
	EarlyItem item = s[i][j];
	for (int k = 0; k < s[item.start].size(); k++) {
		Symbol* nextSym = nextSymbol(g, s[item.start][k]);
		if (nextSym != nullptr && nextSym->match(g.rules[item.rule].name)) {
			// EarlyItem: {rule, next, start}
			if (appendItem(s[i], { s[item.start][k].rule,
							   s[item.start][k].next + 1,
							   s[item.start][k].start }))
				++size;
		}
	}
}

void egp::scan(EarlyVec& s, int i, int j, int& size, Symbol* symbol, const std::string& input)
{
	if (i >= input.length())
		return;

	EarlyItem item = s[i][j];
	if (symbol->match(input.substr(i, 1))) {
		if (i + 1 > s.size() - 1) {
			s.push_back({});
			++size;
		}

		// EarlyItem: {rule, next, start}
		s[i + 1].push_back({ item.rule, 
							 item.next + 1, 
							 item.start }); 
	}
}

void egp::predict(EarlyVec& s, int i, int j, int& size, Symbol* symbol, const Grammar& g, std::unordered_set<std::string>& nss)
{
	for (int k = 0; k < g.rules.size(); k++) {
		if (symbol->match(g.rules[k].name)) {
			// EarlyItem: {rule, next, start}
			if (appendItem(s[i], { k, 0, i }))
				++size;
			if (nss.find(g.rules[k].name) != nss.end()) { // magical completion
				if (appendItem(s[i], { s[i][j].rule, s[i][j].next + 1, s[i][j].start }))
					++size;
			}
		}
	}
}

bool egp::appendItem(std::vector<EarlyItem>& items, EarlyItem item)
{
	std::vector<EarlyItem>::iterator it;
	for (it = items.begin(); it != items.end(); it++) {
		if (*it == item)
			return false;
	}
	items.push_back(item);
	return true;
}

std::unordered_set<std::string> egp::getNullableRules(const Grammar& g) 
{
	std::unordered_set<std::string> nullableSet;
	std::size_t oldSize = 0;
	do {
		oldSize = nullableSet.size();
		updateNullableSet(nullableSet, g);
	} while (oldSize != nullableSet.size());
	return nullableSet;
}

void egp::updateNullableSet(std::unordered_set<std::string>& nullableSet, const Grammar& g)
{
	for (int i = 0; i < g.rules.size(); i++) {
		if (isNullable(g.rules[i], nullableSet))
			nullableSet.insert(g.rules[i].name);
	}
}

bool egp::isNullable(const Rule& rule, const std::unordered_set<std::string>& nullableSet) 
{
	for (int i = 0; i < rule.definition.size(); i++) {
		if (nullableSet.find(rule.definition[i]->toString()) == nullableSet.end())
			return false;
	}
	return true;
}


void egp::printEarlyVec(const EarlyVec& s, const Grammar& g, bool hideIncomplete)
{
	struct Line { std::string name, definition, start; };
	std::vector<std::vector<Line>> lines;
	std::size_t maxNameLen = 0, maxDefLen = 0;

	for (int i = 0; i < s.size(); i++) {
		lines.push_back(std::vector<Line>());

		for (int j = 0; j < s[i].size(); j++) {
			Line l;
			EarlyItem item = s[i][j];
			if (item.rule > -1) {
				Rule rule = g.rules[item.rule];
				l.name = rule.name;
				if (l.name.length() > maxNameLen)
					maxNameLen = l.name.length();

				std::stringstream ruleDef;
				int defSize = rule.definition.size();
				for (int k = 0; k < defSize; k++) {
					if (k == item.next) ruleDef << " @";
					if (typeid(*rule.definition[k]) == typeid(Terminal))
						ruleDef << " " << *rule.definition[k];
					else if (typeid(*rule.definition[k]) == typeid(NonTerminal))
						ruleDef << " " << *rule.definition[k];
					else throw("impossible symbol");
				}
				if (item.next >= defSize) ruleDef << " @";
				else if (hideIncomplete) continue;

				l.definition = ruleDef.str();
				if (l.definition.length() > maxDefLen)
					maxDefLen = l.definition.length();
			}
			else {
				l.definition = "{" + std::to_string(item.rule) + ", " + std::to_string(item.next) + "}";
			}
			
			l.start = "(" + std::to_string(item.start) + ")\n";
			lines.back().push_back(l);
		}
	}

	for (int i = 0; i < lines.size(); i++) {
		std::cout << "    === " << i << " ===\n";
		for (int j = 0; j < lines[i].size(); j++) {
			Line l = lines[i][j];
			std::cout << std::left << std::setw(maxNameLen) << l.name;
			std::cout << " -> " << std::left << std::setw(maxDefLen) << l.definition;
			std::cout << " " << l.start;
		}
		if (i != lines.size() - 1)
			std::cout << std::endl;
	}
}
