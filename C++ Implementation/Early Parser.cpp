// Early Parser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "Terminal.h"
#include "NonTerminal.h"
#include "GrammarInterpreter.h"
#include <algorithm>
#include "GrammarParser.h"

void testInterpreter();

void testMemoryLeak()
{
    std::string input = "Sum -> Sum [Test|Terminals] Product | Product";
    input.erase(remove(input.begin(), input.end(), ' '), input.end());

    for (int i = 0; i < 1000; ++i) {
        egp::EarlyVec s = egp::buildItems(gi::interpreterGrammar, input);
        egp::EarlyVec inverted = egp::invertEarlyVec(s, gi::interpreterGrammar);
        egp::sortEarlyVec(inverted);
        egp::ParseNode* root = egp::buildParseTree(input, inverted, gi::interpreterGrammar);
        egp::ParseNode* actionTree = egp::applySemanticActions(root, gi::interpreterActions);
        egp::deleteParseTree(actionTree);
        egp::deleteParseTree(root);
    }
}

int main()
{
    //testMemoryLeak();
   // testInterpreter();
    egp::Grammar g1 = {
        "Sum",
        {
            {
                "Sum",
                { new NonTerminal("Sum"),
                  new Terminal(std::set<std::string>({"+", "-"})),
                  new NonTerminal("Product") }
            },

            {
                "Sum",
                { new NonTerminal("Product") }
            },

            {
                "Product",
                { new NonTerminal("Product"),
                  new Terminal(std::set<std::string>({"*", "/"})),
                  new NonTerminal("Factor") }
            },

            {
                "Product",
                { new NonTerminal("Factor") }
            },

            {
                "Factor",
                { new Terminal("("),
                  new NonTerminal("Sum"),
                  new Terminal(")") }
            },

            {
                "Factor",
                { new NonTerminal("Number") }
            },

            /*{
                "Number",
                { new Terminal(std::set<std::string>({"0", "1", "2", 
                                                      "3", "4", "5", 
                                                      "6", "7", "8", "9"})),
                  new NonTerminal("Number")}
            },*/

            { 
                "Number",
                {new Terminal(std::set<std::string>({"0", "1", "2",
                                                      "3", "4", "5",
                                                      "6", "7", "8", "9"}))}
            }
        }
    };

    egp::Grammar ifBlock = {
        "If",
        {
            {
                "If",
                { new Terminal("i"),
                  new Terminal("f"),
                  new NonTerminal("Block"),
                  new Terminal("e"),
                  new Terminal("l"),
                  new Terminal("s"),
                  new Terminal("e"),
                  new NonTerminal("Block") }
            },

            {
                "If",
                { new Terminal("i"),
                  new Terminal("f"),
                  new NonTerminal("Block") }
            },

            {
                "Block",
                { new NonTerminal("If") }
            },

            {
                "Block",
                { new Terminal("{"),
                  new Terminal("}"), }
            }
        }
    };

    std::function<egp::ParseNode* (const egp::ParseNode* const)> passChild1;
    passChild1 = [](const egp::ParseNode* const node) -> egp::ParseNode* {
        return node->children[0];
    };

    std::function<egp::ParseNode* (const egp::ParseNode* const)> filterChild02;
    filterChild02 = [](const egp::ParseNode* const node) -> egp::ParseNode* {
        return new egp::ParseNode(node->rule, node->label, {node->children[1]});
    };

    std::function<egp::ParseNode* (const egp::ParseNode* const)> filterRule;
    filterRule = [](const egp::ParseNode* const node) -> egp::ParseNode* {
        return new egp::ParseNode(node->rule, node->label, { node->children[0], node->children[3] });
    };


    std::function<egp::ParseNode* (const egp::ParseNode* const)> nothing;
    nothing = [](const egp::ParseNode* const node) -> egp::ParseNode* {
        return new egp::ParseNode(*node);
    };

    std::function<egp::ParseNode* (const egp::ParseNode* const)> recursiveReducer;
    recursiveReducer = [](const egp::ParseNode* const node) -> egp::ParseNode* {
        std::string temp = node->children[0]->label + node->children[1]->label;
        return new egp::ParseNode(-1, temp);
    };

    std::function<egp::ParseNode* (const egp::ParseNode* const)> reduceExpression;
    reduceExpression = [](const egp::ParseNode* const node) -> egp::ParseNode* {
        //std::vector<egp::ParseNode*> returnVec = { nodes[0] };
        //returnVec.insert(returnVec.end(), nodes[1]->children.begin(), nodes[1]->children.end());
        //return returnVec;
        std::vector<egp::ParseNode*> children = { node->children[0] };
        children.insert(children.end(), 
                        node->children[1]->children.begin(), 
                        node->children[1]->children.end());
        return new egp::ParseNode(node->rule, node->label, children);
    };

    std::function<egp::ParseNode* (const egp::ParseNode* const)> reduceJointExpression;
    reduceJointExpression = [](const egp::ParseNode* const node) -> egp::ParseNode* {
        //std::vector<egp::ParseNode*> returnVec = { nodes[0] };
        //returnVec.insert(returnVec.end(), nodes[1]->children.begin(), nodes[1]->children.end());
        //return returnVec;
        std::vector<egp::ParseNode*> children = { node->children[0] };
        children.insert(children.end(),
            node->children[2]->children.begin(),
            node->children[2]->children.end());
        return new egp::ParseNode(node->rule, node->label, children);
    };

    std::vector<std::function<egp::ParseNode* (const egp::ParseNode* const)>> actions;
    actions.push_back(filterRule);
    actions.push_back(filterRule);
    actions.push_back(reduceJointExpression);
    actions.push_back(nothing);
    actions.push_back(reduceExpression);
    actions.push_back(reduceExpression);
    actions.push_back(nothing); // 7
    actions.push_back(nothing);
    actions.push_back(nothing);
    actions.push_back(filterChild02); // 10
    actions.push_back(filterChild02); // 9
    actions.push_back(passChild1);
    actions.push_back(recursiveReducer);
    actions.push_back(passChild1);
    actions.push_back(recursiveReducer);
    actions.push_back(recursiveReducer);
    actions.push_back(passChild1);
    actions.push_back(passChild1);
    actions.push_back(passChild1); // 19
    actions.push_back(passChild1); // 19

    egp::Grammar g = {
        "A",
        {
            {
                "A",
                { new Terminal("a"),
                  new NonTerminal("A") }
            },

            {
                "A",
                { }
            }
        }
    };

    if (true) {
        std::string input = "aaaa";//"1+(2*3+4)";//"if if {} else {}";
        input.erase(remove(input.begin(), input.end(), ' '), input.end());

        egp::EarlyVec s = egp::buildItems(g, input);

        std::cout << "Input: " << input << std::endl;
        egp::printEarlyVec(s, g);
        std::cout << "=============================================\n";
        egp::EarlyVec inverted = egp::invertEarlyVec(s, g);
        egp::sortEarlyVec(inverted);
        egp::printEarlyVec(inverted, g);
        std::cout << "=============================================\n";
        egp::ParseNode* root = egp::buildParseTree(input, inverted, g);
        egp::printParseTree(root);

        std::cout << "=============================================\n";
        //egp::ParseNode* actionTree = egp::applySemanticActions(root, actions);
        //egp::printParseTree(actionTree, true);
    }
    else {
        std::string input1 = "Sum -> Sum [Test|Terminals] Product | Product";
        std::string input2 = "Sum -> Sum \"*\" Product | ProDuct | Product Sum | AB";
        std::string input = "If -> \"if\" Block \"else\" Block";
        input.erase(remove(input.begin(), input.end(), ' '), input.end());

        egp::EarlyVec s = egp::buildItems(gi::interpreterGrammar, input);

        std::cout << "Input: " << input << std::endl;
        egp::printEarlyVec(s, gi::interpreterGrammar);
        std::cout << "=============================================\n";
        egp::EarlyVec inverted = egp::invertEarlyVec(s, gi::interpreterGrammar);
        egp::sortEarlyVec(inverted);
        egp::printEarlyVec(inverted, gi::interpreterGrammar);
        std::cout << "=============================================\n";
        egp::ParseNode* root = egp::buildParseTree(input, inverted, gi::interpreterGrammar);
        egp::printParseTree(root, true);

        std::cout << "=============================================\n";
        egp::ParseNode* actionTree = egp::applySemanticActions(root, gi::interpreterActions);
        egp::printParseTree(actionTree, true);

        gi::interpretRule(actionTree);
    }
    
}


// Memory Leak Galore. Exploratory Programming so I don't care.
void testInterpreter()
{
    std::cout << "Enter starting rule name: ";

    std::string startingRule;
    std::cin >> startingRule;
    std::cin.get();
    std::cout << std::endl;

    egp::Grammar grammar;
    grammar.startRule = startingRule;

    std::cout << "Enter Rules (enter \"X\" to finish)\n";
    
    int ruleCount = 1;
    std::string input;

    do {
        std::cout << ruleCount << ": ";
        std::getline(std::cin, input);
        input.erase(remove(input.begin(), input.end(), ' '), input.end());

        if (input == "X" || input == "x")
            continue;

        egp::EarlyVec s = egp::buildItems(gi::interpreterGrammar, input);
        egp::EarlyVec inverted = egp::invertEarlyVec(s, gi::interpreterGrammar);
        egp::sortEarlyVec(inverted);
        egp::ParseNode* root = egp::buildParseTree(input, inverted, gi::interpreterGrammar);

        if (root == nullptr) {
            std::cout << "Error: invalid rule!" << std::endl << std::endl;
            continue;
        }

        egp::ParseNode* actionTree = egp::applySemanticActions(root, gi::interpreterActions);
        try {
            std::vector<egp::Rule> newRules = gi::interpretRule(actionTree);
            grammar.rules.insert(grammar.rules.end(), newRules.begin(), newRules.end());
            ruleCount++;
        }
        catch (std::string e) {
            std::cout << "Error: " << e << std::endl << std::endl;
        }

    } while (input != "X" && input != "x");

    do {
        std::cout << "\n=================================\n\n";
        std::cout << "Enter Test Input: ";
        std::getline(std::cin, input);
        std::cout << std::endl;

        if (input == "X" || input == "x")
            continue;

        egp::EarlyVec s = egp::buildItems(grammar, input);
        egp::EarlyVec inverted = egp::invertEarlyVec(s, grammar);
        egp::sortEarlyVec(inverted);
        egp::ParseNode* root = egp::buildParseTree(input, inverted, grammar);

        if (root == nullptr) {
            std::cout << "Error: invalid input!" << std::endl << std::endl;
            continue;
        }

        egp::printParseTree(root, true);

    } while (input != "X" && input != "x");
}