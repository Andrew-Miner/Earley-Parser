#pragma once
#include <set>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>

class Symbol
{
public:
	Symbol() {};
	Symbol(std::string symbol) { symbols.insert(symbol); };
	Symbol(std::set<std::string> symbols) { this->symbols = symbols; };

	virtual bool match(const std::string& symbol) const {
		if (symbols.find(symbol) != symbols.end())
			return true;
		return false;
	};

	virtual bool match(const std::set<std::string>& symbols) const {
		for (auto it = symbols.cbegin(); it != symbols.cend(); ++it) {
			if (this->symbols.find(*it) == this->symbols.cend())
				return false;
		}
		return true;
	}

	std::string toString() const {
		if (symbols.size() > 1) {
			std::string str = "[";
			std::set<std::string>::iterator it;
			for (it = symbols.begin(); it != symbols.end(); it++) {
				if (it != --symbols.end())
					str += *it + ",";
				else
					str += *it;
			}
			str += "]";
			return str;
		}

		return *symbols.begin();
	}

	friend std::ostream& operator<<(std::ostream& os, const Symbol& symbol) {
		os << symbol.toString();
		return os;
	}

protected:
	std::set<std::string> symbols;
};

