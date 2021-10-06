# Earley-Parser
An [Earley Parser](https://en.wikipedia.org/wiki/Earley_parser) implemented in both C++ and Javascript. The C++ implementation needs some work to make it practical for use in other projects.

This Earley Parser consists of two main components. The Recogniser ([`EarleyRecognizer.js`](https://github.com/Andrew-Miner/Earley-Parser/blob/main/Javascript%20Implementation/Earley%20Parser/EarleyRecognizer.js)) and the Parser ([`EarleyParser.js`](https://github.com/Andrew-Miner/Earley-Parser/blob/main/Javascript%20Implementation/Earley%20Parser/EarleyParser.js)). The Recogniser determines if the input is valid and stores information about partial parses in tables. These tables are then handed off to the Parser. The Parser uses these tables to construct an abstract syntax tree and then applies Semantic Actions given by the user to "interpret" the given input.

In order to use the parser, it must first be given a [Context Free Grammar](https://en.wikipedia.org/wiki/Context-free_grammar) and the string to be parsed. The context free grammar is represented as a list of rules, where each rule is made up of a name and a list of Terminal and NonTerminal objects. 

To simplify the construction of Context Free Grammars and to avoid having to write them out in JavaScript, the JS implementation includes BNF interpreter functions ([`BNFInterpreter.js`](https://github.com/Andrew-Miner/Earley-Parser/blob/main/Javascript%20Implementation/Earley%20Parser/BNFInterpreter.js)) that use a Grammar and set of Semantic Actions to construct Context Free Grammars from strings in [Backus-Naur form](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form).

## Live Demo
I use this implementation to parse boolean expressions for my [Boolean Expression Simplifier](https://andrew-miner.github.io/Bool-Simplifier-Website/) single-page React App. You can find the [github repository with the source code](https://github.com/Andrew-Miner/Bool-Simplifier-Website/tree/master/src) on my profile.

## Javascript Examples

Using the BNFInterpreter to construct a Context Free Grammar from a string in Backus-Naur form:
```javascript
import Recognizer from "../Earley Parser/EarleyRecognizer";
import Parser, { ParseNode } from "../Earley Parser/EarleyParser";
import { bnfGrammar, bnfActions, interpretBNF } from "../Earley Parser/BNFInterpreter";

const ARITHMETIC_STR =
        '<sum>      ::= <sum> "+" <product>     |   <sum> "-" <product>     |   <product>\n' +
        '<product>  ::= <product> "*" <factor>  |   <product> "/" <factor>  |   <factor>\n' +
        '<factor>   ::= "(" <sum> ")"           |   <number>\n' +
        "<number>   ::= <number> <digit>        |   <digit>\n" +
        '<digit>   ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"\n';

var arithmeticGrammar;
try {
        let grammar = ARITHMETIC_STR.replaceAll("\\n", "\n");
        let s = Recognizer.buildItems(grammar, bnfGrammar);
        let invertedS = Parser.invertEarleySets(s, bnfGrammar);
        let parseTree = Parser.buildParseTree(grammar, invertedS, bnfGrammar);

        if (parseTree === null) {
            throw new Error("invalid grammar");
        }

        let simplifiedTree = Parser.applySemanticAction(
            (token) => new ParseNode(-1, token),
            parseTree,
            bnfActions
        );

        arithmeticGrammar = interpretBNF(simplifiedTree);
} catch (error) {
        console.log(error);
        arithmeticGrammar = null;
}
```
In this example the string `ARITHMETIC_STR` is a language representing any arithmetic equation, such as `18/(3-7)+2*5`.

We can now use the `arithmeticGrammar` we just constructed to parse and interpret the value of the string `18/(3-7)+2*5`:
```javascript
import Recognizer from "../Earley Parser/EarleyRecognizer";
import Parser, { ParseNode } from "../Earley Parser/EarleyParser";

let semanticActions = [
    (lOperand, op, rOperand) => lOperand + op + rOperand,
    (lOperand, op, rOperand) => lOperand + op + rOperand,
    (product) => product,
    (lOperand, op, rOperand) => lOperand + op + rOperand,
    (lOperand, op, rOperand) => lOperand + op + rOperand,
    (factor) => factor,
    (lParen, sum, rParen) => sum,
    (number) => number,
    (num1, num2) => num1 + num2,
    (digit) => digit,
    (d) => d, (d) => d, (d) => d, (d) => d, (d) => d, (d) => d, (d) => d, (d) => d, (d) => d, (d) => d,
];

let input = "18/(3-7)+2*5";
let arithS = Recognizer.buildItems(input, arithmeticGrammar);
let arithInverted = Parser.invertEarleySets(arithS, arithmeticGrammar);
let arithParseTree = Parser.buildParseTree(input, arithInverted, arithmeticGrammar);

var solution;
if (arithParseTree === null) {
    console.log("Invalid input!");
    solution = null;
} else {
    solution = Parser.applySemanticAction((token) => token, arithParseTree, semanticActions);
}
```
Once this code has been successfully ran, the value of the variable `solution` should be `5.5`. 

Note: for every rule in the arithmetic grammar there is a corresponding semantic action. These semantic actions are called while the Earley Parser recursively walks the parse tree. It hands the output of one semantic action to the input of the next semantic action.
