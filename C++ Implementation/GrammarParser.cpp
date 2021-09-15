#include "GrammarParser.h"
#include <cassert>
#include "GrammarInterpreter.h"
using namespace egp;

void egp::sortEarlyVec(EarlyVec& s)
{
	int sSize = s.size();
	for (int i = 0; i < sSize; i++) {
		std::stable_sort(s[i].begin(), s[i].end(), egp::compareStart);
	}
}

EarlyVec egp::invertEarlyVec(const EarlyVec& s, const Grammar& g, bool filterIncomplete)
{
	EarlyVec inverted;
	padEarlyVec(s.size(), inverted);

	int sSize = s.size();
	for (int i = 0; i < sSize; i++) {
		int itemsSize = s[i].size();
		for (int j = 0; j < itemsSize; j++) {
			EarlyItem item = s[i][j];
			Rule rule = g.rules[item.rule];

			if (filterIncomplete && rule.definition.size() > item.next)
				continue;

			int newSet = item.start;
			item.start = i;
			appendEarlyItem(newSet, item, inverted);
		}
	}
	return inverted;
}

void egp::appendEarlyItem(int set, EarlyItem& item, EarlyVec& s)
{
	if (set >= s.size())
		padEarlyVec(set + 1 - s.size(), s);
	s[set].push_back(item);
}

void egp::padEarlyVec(int amount, EarlyVec& s)
{
	for (int i = 0; i < amount; i++) {
		s.push_back({});
	}
}

ParseNode* egp::buildParseTree(const std::string& input, const EarlyVec& invertedS, const Grammar& g)
{
	std::vector<EarlyItem> completeItems = getEdges(0, input.length(), invertedS);
	if (!completeItems.size())
		return nullptr;

	Edge<int> startingEdge = { 0, input.length(), completeItems[0].rule };
	ParseNode* root = new ParseNode(startingEdge.data, g.rules[startingEdge.data].name);

	// Recursive Nested Function
	std::function<void(const Edge<int>&, ParseNode*)> buildTree;
	buildTree = [&input, &invertedS, &g, &buildTree](const Edge<int>& edge, ParseNode* root) {
		std::vector<Edge<int>> children = decomposeEdge(input, invertedS, g, edge);
		for (auto it = children.begin(); it != children.end(); ++it) {
			if (it->data == -1) {
				root->children.push_back(new ParseToken(input.substr(it->startNode, 1)));
			}
			else {
				ParseNode* newNode = new ParseNode(it->data, g.rules[it->data].name);
				root->children.push_back(newNode);
				buildTree(*it, newNode);
			}
		}
	};

	buildTree(startingEdge, root);
	return root;
}

std::vector<Edge<int>> egp::decomposeEdge(const std::string& input, const EarlyVec& graph, const Grammar& g, const Edge<int>& edge)
{
	assert(edge.startNode < graph.size());					// assertion that start node is within the bounds of the graph
	assert(edge.endNode < graph.size());					// assertion that end node is within the bounds of the graph
	assert(edge.data >= 0 && edge.data < g.rules.size());	// assertion that edge.data is a valid rule index

	const std::vector<Symbol*>& rules = g.rules[edge.data].definition;

	int start = edge.startNode;
	int finish = edge.endNode;

	int bottom = rules.size();	// Each symbol represents one edge.
								// So the number of found subEdges
								// should be the number of symbols.
								// Therefore the max depth is the
								// number of symbols.


	// These nested functions are an attempt at making the depth first search
	// function generic and also minimize the parameters these functions take 
	auto isLeaf = [&finish, &bottom](int node, int depth) -> bool {
		return node == finish && depth == bottom;
	};

	auto getChild = [](const Edge<int>& edge, int depth) -> int {
		return edge.endNode;
	};

	auto getEdges = [&edge, &rules, &input, &graph, &g](int node, int depth) -> std::vector<Edge<int>> {
		if (depth >= rules.size())
			return {};

		std::vector<Edge<int>> edges;
		Symbol* symbol = rules[depth];

		// If we are dealing with a Terminal we
		// don't need to iterate the graph because
		// it is missing all Terminal/Scan edges
		if (typeid(*symbol) == typeid(Terminal)) {
			if (symbol->match(input.substr(node, 1)))
				edges.push_back({ node, node + 1, -1});
		}
		else if (typeid(*symbol) == typeid(NonTerminal)) {
			for (auto it = graph[node].begin(); it != graph[node].end(); ++it) {
				if (symbol->match(g.rules[it->rule].name))
					edges.push_back({ node, it->start, it->rule });
			}
		}

		return edges;
	};

	return depthFirstSearch<int>(start, getEdges, isLeaf, getChild);
}

std::vector<EarlyItem> egp::getEdges(int startNode, int endNode, const EarlyVec& graph) 
{
	assert(graph.size() > startNode);

	std::vector<EarlyItem> edges;
	for (auto it = graph[startNode].begin(); it != graph[startNode].end(); ++it) {
		// start is really the end node because 
		// the graph is an inverted EarlySet
		if (it->start == endNode)	
			edges.push_back(*it);
	}
	return edges;
}

void egp::printParseTree(ParseNode* node, bool printRule)
{
	std::function<void(ParseNode*, std::string indent, bool last)> recursivePrint;
	recursivePrint = [&printRule, &recursivePrint](ParseNode* node, std::string indent, bool last) {
		std::cout << indent << "+- " << node->label;
		if (printRule)
			std::cout << " (" << node->rule << ")" << std::endl;
		else
			std::cout << std::endl;

		indent += last ? "   " : "|  ";

		for (std::size_t i = 0; i < node->children.size(); i++) {
			recursivePrint(node->children[i], indent, i == node->children.size() - 1);
		}
	};

	recursivePrint(node, "", true);
}

void egp::deleteParseTree(ParseNode* node)
{
	for (auto it = node->children.begin(); it != node->children.end(); it++)
		deleteParseTree(*it);
	delete node;
}


ParseNode* egp::applySemanticActions(const ParseNode* const tree, const std::vector<std::function<ParseNode*(const ParseNode&)>>& actions)
{
	std::function<ParseNode* (const ParseNode* const)> traverseBottomUp;
	traverseBottomUp = [&actions, &traverseBottomUp](const ParseNode* const sourceNode) -> ParseNode* {
		if (sourceNode->children.size() > 0) {
			std::vector<ParseNode*> newChildren;
			for (ParseNode* child : sourceNode->children)
				newChildren.push_back(traverseBottomUp(child));

			ParseNode tempNode = { sourceNode->rule, sourceNode->label, newChildren };
			return actions[sourceNode->rule](tempNode);
		}
		else
			return new ParseNode(sourceNode->rule, sourceNode->label);

	};

	return traverseBottomUp(tree);
}

/*ParseNode* egp::applySemanticActions(const ParseNode* const tree, const ActionVec& actions)
{
	std::function<ParseNode*(const ParseNode* const)> traverseBottomUp;
	traverseBottomUp = [&actions, &traverseBottomUp](const ParseNode* const sourceNode) -> ParseNode* {
		if (sourceNode->children.size() != 0) {	// If NonTerminal/Not Leaf Node
			std::vector<ParseNode*> newChildren;
			for (ParseNode* child : sourceNode->children) {
				newChildren.push_back(traverseBottomUp(child));
			}

			if (sourceNode->label == "Rule")
				std::cout << "Rule = " << sourceNode->rule << std::endl;
			std::vector<ParseNode*> actionChildren = actions[sourceNode->rule](newChildren);
			return new ParseNode(sourceNode->rule, sourceNode->label, actionChildren);
		}
		else
			return new ParseNode(sourceNode->rule, sourceNode->label);
	};

	return traverseBottomUp(tree);
}*/