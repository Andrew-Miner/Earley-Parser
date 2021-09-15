#pragma once
#include "GrammarRecognizer.h"
#include "NonTerminal.h"
#include "Terminal.h"
#include <functional>
#include <initializer_list>

namespace egp
{
	// Graph Example: http://graphonline.ru/en/?graph=MpqftqFDbTdJGcDy
	struct ParseNode
	{
		int rule = -1;
		std::string label;
		std::vector<ParseNode*> children;

		ParseNode() {}
		ParseNode(int rule, std::string label) : rule(rule), label(label) {}
		ParseNode(int rule, std::string label, std::vector<ParseNode*> children)
			: rule(rule), label(label), children(children) {}
	};

	struct ParseToken : public ParseNode
	{
		ParseToken() : ParseNode() {}
		ParseToken(std::string label) : ParseNode(-1, label) {}
	};

	template<typename T>
	struct Edge
	{
		int startNode, endNode;
		T data;
	};


	// ActionFuncs take in a temporary ParseNode and returns a new ParseNode allocated
	// on the heap. ActionFuncs serve as semantic actions that are applied to
	// a parse tree. The Node passed into an ActionFunc holds the
	// rule associated with the ActionFunc. Even if an ActionFunc does nothing 
	// and just returns the node it was passed, the node returned must be a copy 
	// of the node passed and must be allocated on the heap. 
	// WARNING: If your semantic action cuts a Parse Node out of the tree, don't
	//			forget to delete that Node. Otherwise a memory leak will occur.
	//			( e.g. if you return 
	//			  "new ParseNode({-1, label, node->children[0]->children});"
	//			  then you must delete node->children[0] to prevent a memory leak )
	typedef std::function<ParseNode* (const ParseNode&)> ActionFunc;
	typedef std::vector < ActionFunc > ActionVec;

	void sortEarlyVec(EarlyVec& s);
	EarlyVec invertEarlyVec(const EarlyVec& s, const Grammar& g, bool filterIncomplete = true);
	void padEarlyVec(int amount, EarlyVec& s);
	void appendEarlyItem(int set, EarlyItem& item, EarlyVec& s);

	ParseNode* buildParseTree(const std::string& input, const EarlyVec& invertedS, const Grammar& g);
	void printParseTree(ParseNode* node, bool printRule = false);
	void deleteParseTree(ParseNode* node);

	std::vector<EarlyItem> getEdges(int startNode, int endNode, const EarlyVec& graph);
	std::vector<Edge<int>> decomposeEdge(const std::string& input, const EarlyVec& graph, const Grammar& g, const Edge<int>& edge);

	template<typename T>
	std::vector<Edge<T>> depthFirstSearch(int root,
									      std::function<std::vector<Edge<T>>(int, int)> getEdges,
									      std::function<bool(int, int)> isLeaf,
									      std::function<int(const Edge<T>&, int)> getChild);


	// Prone To Memory Leaks.
	// This is a quick and very sloppy implementation that needs
	// to be modified in order to prevent memory leaks. 
	// Also heavily uses shared pointers in an extremely bad way.
	// WARNING: If your semantic action cuts a Parse Node out of the tree, don't
	//			forget to delete that Node. Otherwise a memory leak will occur.
	ParseNode* applySemanticActions(const ParseNode* const tree, const ActionVec& actions);
}

// Generic Depth First Search
template<typename T>
std::vector<egp::Edge<T>> egp::depthFirstSearch(int root,
												std::function<std::vector<Edge<T>>(int, int)> getEdges,
												std::function<bool(int, int)> isLeaf,
												std::function<int(const Edge<T>&, int)> getChild)
{
	std::vector<Edge<T>> path;
	std::function<bool(int, int)> dfs;
	dfs = [&isLeaf, &getEdges, &getChild, &path, &dfs](int node, int depth)->bool {
		if (isLeaf(node, depth))
			return true;
		auto edges = getEdges(node, depth);
		for (auto it = edges.begin(); it != edges.end(); ++it) {
			int child = getChild(*it, depth);
			if (dfs(child, depth + 1)) {
				path.insert(path.begin(), *it);
				return true;
			}
		}
		return false;
	};

	dfs(root, 0);
	return path;
}
