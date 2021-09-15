#pragma once
#include "Symbol.h"

class Terminal : public Symbol
{
public:
	Terminal(): Symbol() {};
	Terminal(const std::string& symbol): Symbol(symbol) {};
	Terminal(std::set<std::string> symbols): Symbol(symbols) {};
};