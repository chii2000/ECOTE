//
// Created by chihi on 2023/04/26.
//
#include <iostream>
#include <fstream>
#include <stack>
#include <queue>
#include <string>
#include <vector>
#include <memory>
#include <cctype>
#include <set>

class State {
public:
    int id;
    char symbol;
    std::vector<int> transitions;

    State(int id, char symbol) : id(id), symbol(symbol) {}
};

class NFA {
public:
    std::vector<std::shared_ptr<State>> states;
    int initialState;
    int finalState;
};

std::string preprocessor(const std::string &regex);
std::string postfix(const std::string &regex);
std::shared_ptr<State> createState(int id, char symbol);
NFA createNFA(char symbol);
NFA concatNFAs(NFA &nfa1, NFA &nfa2);
NFA orNFAs(NFA &nfa1, NFA &nfa2);
NFA closureNFA(NFA &nfa);
NFA positiveNFA(NFA &nfa);
NFA thompson(const std::string &postfixRegex);
void epsilonClosure(const NFA &nfa, int state, std::set<int> &closure);
bool accepts(NFA &nfa, const std::string &input);
bool isValidRegex(const std::string &regex);
void printNFA(const NFA &nfa);

int main(int argc, char *argv[]) {
    std::string regex;
    if (argc == 2) {
        std::ifstream file(argv[1]);
        if (file) {
            std::getline(file, regex);
            file.close();
        } else {
            std::cerr << "Error: Unable to open the file." << std::endl;
            return 1;
        }
    } else {
        std::cout << "Enter the regular expression: ";
        std::getline(std::cin, regex);
    }

    if (!isValidRegex(regex)) {
        std::cerr << "Error: Invalid regular expression." << std::endl;
        return 1;
    }

    std::string preprocessedRegex = preprocessor(regex);
    std::cout << "Preprocessed: " << preprocessedRegex << std::endl;
    std::string postfixRegex = postfix(preprocessedRegex);
    std::cout << "Postfix: " << postfixRegex << std::endl;
    NFA nfa = thompson(postfixRegex);

    printNFA(nfa);

    std::string input;
    while (true) {
        std::cout << "Enter the input string (exit to quit): ";
        std::getline(std::cin, input);
        if (input == "exit") break;
        std::cout << (accepts(nfa, input) ? "Accepted" : "Rejected") << std::endl;
    }

    return 0;
}

std::string preprocessor(const std::string &regex) {
    std::string result;
    char prev = 0;

    for (char c : regex) {
        if (prev != 0 && prev != '(' && prev != '|' && c != ')' && c != '|' && c != '*' && c != '+') {
            result.push_back('.');
        }
        result.push_back(c);
        prev = c;
    }

    return result;
}

std::string postfix(const std::string &regex) {
    std::string result;
    std::stack<char> opStack;

    for (char c : regex) {
        if (c == '(') {
            opStack.push(c);
        } else if (c == ')') {
            while (!opStack.empty() && opStack.top() != '(') {
                result.push_back(opStack.top());
                opStack.pop();
            }
            if (!opStack.empty()) {
                opStack.pop();
            }
        } else if (c == '|' || c == '.' || c == '*' || c == '+') {
            while (!opStack.empty() && opStack.top() != '(') {
                result.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(c);
        } else {
            result.push_back(c);
        }
    }

    while (!opStack.empty()) {
        result.push_back(opStack.top());
        opStack.pop();
    }

    return result;
}

std::shared_ptr<State> createState(int id, char symbol) {
    return std::make_shared<State>(id, symbol);
}

NFA createNFA(char symbol) {
    NFA nfa;
    nfa.states.push_back(createState(0, symbol));
    nfa.states.push_back(createState(1, 'E')); // Epsilon transition
    nfa.states[0]->transitions.push_back(1);
    nfa.initialState = 0;
    nfa.finalState = 1;

    return nfa;
}

NFA concatNFAs(NFA &nfa1, NFA &nfa2) {
    NFA result;
    int offset = nfa1.states.size();
    for (const auto &state : nfa1.states) {
        result.states.push_back(state);
    }

    for (const auto &state : nfa2.states) {
        auto newState = createState(state->id + offset, state->symbol);
        for (int trans : state->transitions) {
            newState->transitions.push_back(trans + offset);
        }
        result.states.push_back(newState);
    }

    result.states[nfa1.finalState]->transitions.push_back(nfa2.initialState + offset);

    result.initialState = nfa1.initialState;
    result.finalState = nfa2.finalState + offset;

    return result;
}

NFA orNFAs(NFA &nfa1, NFA &nfa2) {
    NFA result;
    int offset1 = 1;
    int offset2 = nfa1.states.size() + 1;
    result.states.push_back(createState(0, 'E'));

    for (const auto &state : nfa1.states) {
        auto newState = createState(state->id + offset1, state->symbol);
        for (int trans : state->transitions) {
            newState->transitions.push_back(trans + offset1);
        }
        result.states.push_back(newState);
    }

    for (const auto &state : nfa2.states) {
        auto newState = createState(state->id + offset2, state->symbol);
        for (int trans : state->transitions) {
            newState->transitions.push_back(trans + offset2);
        }
        result.states.push_back(newState);
    }

    result.states.push_back(createState(result.states.size(), 'E'));

    result.states[0]->transitions.push_back(nfa1.initialState + offset1);
    result.states[0]->transitions.push_back(nfa2.initialState + offset2);

    result.states[nfa1.finalState + offset1]->transitions.push_back(result.states.size() - 1);
    result.states[nfa2.finalState + offset2]->transitions.push_back(result.states.size() - 1);

    result.initialState = 0;
    result.finalState = result.states.size() - 1;

    return result;
}

NFA closureNFA(NFA &nfa) {
    NFA result;
    int offset = 1;
    result.states.push_back(createState(0, 'E'));

    for (const auto &state : nfa.states) {
        auto newState = createState(state->id + offset, state->
                symbol);
        for (int trans : state->transitions) {
            newState->transitions.push_back(trans + offset);
        }
        result.states.push_back(newState);
    }
    result.states.push_back(createState(result.states.size(), 'E'));

    result.states[0]->transitions.push_back(nfa.initialState + offset);
    result.states[0]->transitions.push_back(result.states.size() - 1);

    result.states[nfa.finalState + offset]->transitions.push_back(nfa.initialState + offset);
    result.states[nfa.finalState + offset]->transitions.push_back(result.states.size() - 1);

    result.initialState = 0;
    result.finalState = result.states.size() - 1;

    return result;
}

NFA positiveNFA(NFA &nfa) {
    NFA result = closureNFA(nfa);
    NFA concat_result = concatNFAs(nfa, result);
    return concat_result;
}

NFA thompson(const std::string &postfixRegex) {
    std::stack<NFA> nfaStack;
    for (char c : postfixRegex) {
        if (c == '.') {
            NFA nfa2 = nfaStack.top();
            nfaStack.pop();
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();

            NFA concat_result = concatNFAs(nfa1, nfa2);
            nfaStack.push(concat_result);
        } else if (c == '|') {
            NFA nfa2 = nfaStack.top();
            nfaStack.pop();
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();

            NFA or_result = orNFAs(nfa1, nfa2);
            nfaStack.push(or_result);
        } else if (c == '*') {
            NFA nfa = nfaStack.top();
            nfaStack.pop();

            NFA closure_result = closureNFA(nfa);
            nfaStack.push(closure_result);
        } else if (c == '+') {
            NFA nfa = nfaStack.top();
            nfaStack.pop();

            NFA positive_result = positiveNFA(nfa);
            nfaStack.push(positive_result);
        } else {
            NFA nfa = createNFA(c);
            nfaStack.push(nfa);
        }
    }

    return nfaStack.top();
}

void epsilonClosure(const NFA &nfa, int state, std::set<int> &closure) {
    closure.insert(state);
    for (int trans : nfa.states[state]->transitions) {
        if (nfa.states[state]->symbol == 'E' && closure.find(trans) == closure.end()) {
            epsilonClosure(nfa, trans, closure);
        }
    }
}

bool accepts(NFA &nfa, const std::string &input) {
    std::set<int> currentStates;
    epsilonClosure(nfa, nfa.initialState, currentStates);

    for (char c : input) {
        std::set<int> nextStates;

        for (int currentState : currentStates) {
            if (nfa.states[currentState]->symbol == c) {
                for (int trans : nfa.states[currentState]->transitions) {
                    epsilonClosure(nfa, trans, nextStates);
                }
            }
        }

        currentStates = nextStates;
    }

    return currentStates.find(nfa.finalState) != currentStates.end();
}


bool isValidRegex(const std::string &regex) {
// Add your regex validation logic here.
    return true;
}

void printNFA(const NFA &nfa) {
    std::cout << "Transition table:" << std::endl;
    for (const auto &state: nfa.states) {
        std::cout << state->id << " (" << state->symbol << "): ";
        for (int trans: state->transitions) {
            std::cout << trans << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Initial state: " << nfa.initialState << std::endl;
    std::cout << "Final state: " << nfa.finalState << std::endl;
}
