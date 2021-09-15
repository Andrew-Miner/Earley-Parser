#include "GrammarInterpreter.h"

using namespace gi;

// Needs to be modified in order to
// allow quotes to be matched as a character
const egp::Grammar gi::interpreterGrammar = {
	"Rule",
	{
		{
			"Rule",
			{ new NonTerminal("NonTerminal"),
			  new Terminal("-"),
			  new Terminal(">"),
			  new NonTerminal("Expression") }
		},

		{
			"Rule",
			{ new NonTerminal("NonTerminal"),
			  new Terminal("-"),
			  new Terminal(">"),
			  new NonTerminal("JointExpression") }
		},

		{
			"JointExpression",
			{ new NonTerminal("Expression"),
			  new Terminal("|"),
			  new NonTerminal("JointExpression") }
		},

		{
			"JointExpression",
			{ new NonTerminal("Expression") }
		},

		{
			"Expression",
			{ new NonTerminal("Terminal"),
			  new NonTerminal("Expression") }
		},

		{
			"Expression",
			{ new NonTerminal("NonTerminal"),
			  new NonTerminal("Expression") }
		},

		{
			"Expression",
			{ new NonTerminal("Terminal") }
		},

		{
			"Expression",
			{ new NonTerminal("NonTerminal") }
		},

		{
			"NonTerminal",
			{ new NonTerminal("Name") }
		},

		{
			"Terminal",
			{ new Terminal("\""),
			  new NonTerminal("Word"),
			  new Terminal("\"") }
		},

		{
			"Terminal",
			{ new Terminal("["),
			  new NonTerminal("Word"),
			  new Terminal("]") }
		},

		{
			"Name",
			{ new Terminal({"A", "B", "C", "D", "E",
							"F", "G", "H", "I", "J",
							"K", "L", "M", "N", "O",
							"P", "Q", "R", "S", "T",
							"U", "V", "W", "X", "Y", "Z"}) }
		},

		{
			"Name",
			{ new NonTerminal("Name"),
			  new NonTerminal("Lowercase") }
		},

		{
			"Word",
			{ new NonTerminal("Letter") }
		},

		{
			"Word",
			{ new NonTerminal("Word"),
			  new NonTerminal("Letter") }
		},

		{
			"Word",
			{ new NonTerminal("Word"),
			  new NonTerminal("Character") }
		},

		{
			"Word",
			{ new NonTerminal("Character") }
		},

		{
			"Letter",
			{ new Terminal({"a", "b", "c", "d", "e",
							"f", "g", "h", "i", "j",
							"k", "l", "m", "n", "o",
							"p", "q", "r", "s", "t",
							"u", "v", "w", "x", "y", "z",
							"A", "B", "C", "D", "E",
							"F", "G", "H", "I", "J",
							"K", "L", "M", "N", "O",
							"P", "Q", "R", "S", "T",
							"U", "V", "W", "X", "Y", "Z"}) }
		},

		{
			"Character",
			{ new Terminal({"+", "-", "!", "@" , "#",
							"$", "%", "^", "&", "*",
							"(", ")", "<", ">", ",",
							".", "/", "?", ":", ";",
							"\'", "{", "}", "-", "_",
							"=", "\\", "`", "~", "|",
							"0", "1", "2", "3", "4",
							"5", "6", "7", "8", "9"}) }
		},

		{
			"Lowercase",
			{ new Terminal({"a", "b", "c", "d", "e",
							"f", "g", "h", "i", "j",
							"k", "l", "m", "n", "o",
							"p", "q", "r", "s", "t",
							"u", "v", "w", "x", "y", "z"}) }
		}
	}
};

const egp::ActionVec gi::interpreterActions = {
	passChildren03, passChildren03, reduceJointExpression,
	noAction, reduceExpression, reduceExpression, noAction, 
	noAction, noAction, passChild1, passChild1, passChild0, 
	combineChildren, passChild0, combineChildren, combineChildren,
	passChild0, passChild0, passChild0, passChild0
};



egp::ParseNode* gi::passChild0(const egp::ParseNode& node) {
	return node.children[0];
}

egp::ParseNode* gi::passChild1(const egp::ParseNode& node) {
	delete node.children[0]; // Bad design, semantic actions
	delete node.children[2]; // shouldn't have to mess with 
							  // pointer creation/deletion

	return new egp::ParseNode(node.rule, node.label, { node.children[1] });
}

egp::ParseNode* gi::passChildren03(const egp::ParseNode& node) {
	delete node.children[1]; // Bad design, semantic actions
	delete node.children[2]; // shouldn't have to mess with 
							  // pointer creation/deletion

	return new egp::ParseNode(node.rule, node.label, { node.children[0], node.children[3] });
}

egp::ParseNode* gi::noAction(const egp::ParseNode& node) {
	return new egp::ParseNode(node);
}

egp::ParseNode* gi::combineChildren(const egp::ParseNode& node) {
	std::string temp = node.children[0]->label + node.children[1]->label;

	delete node.children[0]; // Bad design, semantic actions
	delete node.children[1]; // shouldn't have to mess with 
							  // pointer creation/deletion

	return new egp::ParseNode(-1, temp);
}

egp::ParseNode* gi::reduceExpression(const egp::ParseNode& node) {
	std::vector<egp::ParseNode*> children = { node.children[0] };
	children.insert(children.end(),
					node.children[1]->children.begin(),
					node.children[1]->children.end());

	delete node.children[1]; // Bad design, semantic actions
							  // shouldn't have to mess with 
							  // pointer creation/deletion

	return new egp::ParseNode(node.rule, node.label, children);
}

egp::ParseNode* gi::reduceJointExpression(const egp::ParseNode& node) {
	std::vector<egp::ParseNode*> children = { node.children[0] };
	children.insert(children.end(),
					node.children[2]->children.begin(),
					node.children[2]->children.end());

	delete node.children[1]; // Bad design, semantic actions
	delete node.children[2]; // shouldn't have to mess with 
							  // pointer creation/deletion

	return new egp::ParseNode(node.rule, node.label, children);
}

std::vector<egp::Rule> gi::interpretRule(const egp::ParseNode* const simplifiedTree)
{
	if (!simplifiedTree || simplifiedTree->label != "Rule")
		throw "invalid parse tree";
	
	egp::ParseNode* ruleNonTerminal = simplifiedTree->children[0];
	if (ruleNonTerminal->label != "NonTerminal" || !ruleNonTerminal->children.size())
		throw "invalid parse tree";

	if (simplifiedTree->children.size() < 2)
		throw "invalid parse tree";

	std::vector<egp::Rule> rules;
	std::string ruleName = ruleNonTerminal->children[0]->label;
	egp::ParseNode* definition = simplifiedTree->children[1];

	if (definition->label == "Expression")
		rules.push_back(buildRule(ruleName, definition));
	else if (definition->label == "JointExpression")
		for (egp::ParseNode* child : definition->children)
			rules.push_back(buildRule(ruleName, child));
	else
		throw "invalid parse tree";

	return rules;
}

egp::Rule gi::buildRule(const std::string& ruleName, const egp::ParseNode* const &expression)
{
	const int NONTERMINAL = 8;
	const int WORD_TERMINAL = 9;
	const int CHAR_TERMINAL = 10;

	if (expression->label != "Expression")
		throw "invalid expression tree";

	std::vector<Symbol*> definition;
	for (egp::ParseNode* child : expression->children) {
		if (child->children.size() < 0 || child->children.size() > 1)
			throw "invalid expression tree";

		if (child->children[0]->rule != -1)
			throw "invalid expression tree";

		std::string token = child->children[0]->label;

		if (child->rule == NONTERMINAL)
			definition.push_back(new NonTerminal(token));
		else if (child->rule == WORD_TERMINAL)
			for (char c : token)
				definition.push_back(new Terminal(std::string(1,c)));
		else if (child->rule == CHAR_TERMINAL) {
			std::set<std::string> tokens;
			for (char c : token)
				tokens.insert(std::string(1,c));
			definition.push_back(new Terminal(tokens));
		}
		else
			throw "invalid expression tree";
	}

	return { ruleName, definition };
}
