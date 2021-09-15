#pragma once
#include "Symbol.h"
class NonTerminal : public Symbol
{
public:
	NonTerminal() : Symbol() {};
	NonTerminal(std::string symbol) : Symbol(symbol) {};
	NonTerminal(std::set<std::string> symbols) : Symbol(symbols) {};
};