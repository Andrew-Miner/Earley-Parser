#pragma once

#include "GrammarRecognizer.h"
#include "Terminal.h"
#include "NonTerminal.h"
#include "GrammarParser.h"

namespace gi 
{
	extern const egp::Grammar interpreterGrammar;
	extern const egp::ActionVec interpreterActions;

    egp::ParseNode* passChild0(const egp::ParseNode& nodes);
    egp::ParseNode* passChild1(const egp::ParseNode& nodes);
    egp::ParseNode* passChildren03(const egp::ParseNode& nodes);
    egp::ParseNode* noAction(const egp::ParseNode& nodes);
    egp::ParseNode* combineChildren(const egp::ParseNode& nodes);
    egp::ParseNode* reduceExpression(const egp::ParseNode& nodes);
    egp::ParseNode* reduceJointExpression(const egp::ParseNode& nodes);

    std::vector<egp::Rule> interpretRule(const egp::ParseNode* const simplifiedTree);
    egp::Rule buildRule(const std::string& ruleName, const egp::ParseNode* const &expression);
}