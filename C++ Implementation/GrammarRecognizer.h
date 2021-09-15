#pragma once
#include <string>
#include <vector>
#include "Symbol.h"
#include <unordered_set>

namespace egp 
{
	struct EarlyItem;
	typedef std::vector<std::vector<EarlyItem>> EarlyVec;

	struct Rule
	{
		std::string name;
		std::vector<Symbol*> definition;
	};

	struct Grammar
	{
		std::string startRule;
		std::vector<Rule> rules;
	};

	struct EarlyItem
	{
		int rule, next, start;
		bool operator==(const EarlyItem& other) {
			return rule == other.rule && 
				   next == other.next && 
				   start == other.start;
		}
	};

	bool compareStart(const EarlyItem& first, const EarlyItem& second);
	EarlyVec buildItems(const Grammar& g, const std::string& input);
	Symbol* nextSymbol(const Grammar& g, const EarlyItem& item);
	void complete(EarlyVec& s, int i, int j, int& size, const Grammar& g);
	void scan(EarlyVec& s, int i, int j, int& size, Symbol* symbol, const std::string& input);
	void predict(EarlyVec& s, int i, int j, int& size, Symbol* symbol, const Grammar& g, std::unordered_set<std::string>& nss);

	bool appendItem(std::vector<EarlyItem>& items, EarlyItem item);
	void printEarlyVec(const EarlyVec& s, const Grammar& g, bool hideIncomplete = false);

	std::unordered_set<std::string> getNullableRules(const Grammar& g);
	void updateNullableSet(std::unordered_set<std::string>& nullableSet, const Grammar& g);
	bool isNullable(const Rule& rule, const std::unordered_set<std::string>& nullableSet);
}

