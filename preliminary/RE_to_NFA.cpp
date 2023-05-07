#include<iostream>
#include<vector>
#include<string>
#include<set>
#include<stack>
#include <algorithm>

using namespace std;

struct trans {
	int vertex_from;
	int vertex_to;
	char trans_symbol;
};

class NFA {
public:
	vector<int> vertex;
	vector<trans> transitions;
	int final_state;

	NFA() {
	}

	int get_vertex_count() {
		return vertex.size();
	}

	void set_vertex(int no_vertex) {
		for(int i = 0; i < no_vertex; i++) {
			vertex.push_back(i);
		}
	}

	void set_transition(int vertex_from, int vertex_to, char trans_symbol) {
		trans new_trans;
		new_trans.vertex_from = vertex_from;
		new_trans.vertex_to = vertex_to;
		new_trans.trans_symbol = trans_symbol;
		transitions.push_back(new_trans);
	}

	void set_final_state(int fs) {
		final_state = fs;
	}

	int get_final_state() {
		return final_state;
	}

	void display() {
		trans new_trans;
		cout<<"\n";
		for(int i = 0; i < transitions.size(); i++) {
			new_trans = transitions.at(i);
			cout<<"q"<<new_trans.vertex_from<<" --> q"<<new_trans.vertex_to<<" : Symbol - "<<new_trans.trans_symbol<<endl;
		}
		cout<<"\nThe final state is q"<<get_final_state()<<endl;
	}
};


NFA* concat(NFA* a, NFA* b) {
	NFA* result = new NFA();
	result->set_vertex(a->get_vertex_count() + b->get_vertex_count() - 1);
	int i;
	trans new_trans;

	for(i = 0; i < a->transitions.size(); i++) {
		new_trans = a->transitions.at(i);
		result->set_transition(new_trans.vertex_from, new_trans.vertex_to, new_trans.trans_symbol);
	}

	for(i = 0; i < b->transitions.size(); i++) {
		new_trans = b->transitions.at(i);
		result->set_transition(new_trans.vertex_from + a->get_vertex_count() - 1, new_trans.vertex_to + a->get_vertex_count() - 1, new_trans.trans_symbol);
	}

	result->set_final_state(a->get_vertex_count() + b->get_vertex_count() - 2);

	return result;
}

NFA* kleene(NFA* a) {
	NFA* result = new NFA();
	int i;
	trans new_trans;
	
	result->set_vertex(a->get_vertex_count() + 2);

	result->set_transition(0, 1, '^');

	for(i = 0; i < a->transitions.size(); i++) {
		new_trans = a->transitions.at(i);
		result->set_transition(new_trans.vertex_from + 1, new_trans.vertex_to + 1, new_trans.trans_symbol);
	}

	result->set_transition(a->get_vertex_count(), a->get_vertex_count() + 1, '^');
	result->set_transition(a->get_vertex_count(), 1, '^');
	result->set_transition(0, a->get_vertex_count() + 1, '^');

	result->set_final_state(a->get_vertex_count() + 1);

	return result;
}

NFA* or_selection(vector<NFA> selections, int no_of_selections) {
	NFA* result = new NFA();
	int vertex_count = 2;
	int i, j;
	NFA med;
	trans new_trans;

	for(i = 0; i < no_of_selections; i++) {
		vertex_count += selections.at(i).get_vertex_count();
	}

	result->set_vertex(vertex_count);
	
	int adder_track = 1;

	for(i = 0; i < no_of_selections; i++) {
		result->set_transition(0, adder_track, '^');
		med = selections.at(i);
		for(j = 0; j < med.transitions.size(); j++) {
			new_trans = med.transitions.at(j);
			result->set_transition(new_trans.vertex_from + adder_track, new_trans.vertex_to + adder_track, new_trans.trans_symbol);
		}
		adder_track += med.get_vertex_count();

		result->set_transition(adder_track - 1, vertex_count - 1, '^');
	}

	result->set_final_state(vertex_count - 1);

	return result;
}

/*
Grammar for regex:
regex = exp $
exp      = term [|] exp
         | term
         |                   empty
term     = factor term
         | factor
factor   = primary [*]
         | primary
primary  = \( exp \)
         | char 
*/
class parser {
    string input;
    int pos;
    vector<int> output;
public:
    parser(string s) {
        input = s;
        pos = 0;
    }

    void primary() {
        if (pos >= input.length()) {
            return;
        }
        if (input[pos] == '(') {
            pos++;
            exp();
            if (input[pos] == ')') {
                pos++;
            } else {
                throw "parse error";
            }
        } else {
            output.push_back(input[pos]);
            pos++;
        }
    }
    void factor() {
        primary();
        if (pos < input.length() && input[pos] == '*') {
            output.push_back(input[pos]);
            pos++;
        }
    }
    void term() {
        factor();
        if (pos < input.length() && input[pos] != ')' && input[pos] != '|') {
            term();
            output.push_back('.'); // concatenation
        }
    }
    void exp() {
        term();
        if (pos < input.length() && input[pos] == '|') {
            pos++;
            exp();
            output.push_back('|');
        }

    }

    void construct(stack<NFA*> &operands) {
    #if 0
        for (vector<int>::iterator it = output.begin() ; it != output.end(); ++it) {
            printf("%c", (char)*it);
        }
        printf("\n");
    #endif
        for (vector<int>::iterator it = output.begin() ; it != output.end(); ++it) {
            int c = *it;
            if (c == '*') {
                NFA* star_sym = operands.top();
				operands.pop();
				operands.push(kleene(star_sym));
            } else if (c == '|') {
                NFA* rhs = operands.top();
				operands.pop();
                NFA* lhs = operands.top();
                operands.pop();

	            vector<NFA> selections(2, NFA());
	            selections.at(0) = *lhs;
        	    selections.at(1) = *rhs;
	            operands.push(or_selection(selections, 2));
            } else if (c == '.' ) {
                NFA* rhs = operands.top();
				operands.pop();
                NFA* lhs = operands.top();
                operands.pop();
                operands.push(concat(lhs, rhs));
            } else {
                NFA* new_sym = new NFA();
			    new_sym->set_vertex(2);
			    new_sym->set_transition(0, 1, c);
			    new_sym->set_final_state(1);
			    operands.push(new_sym);
            }
        }
    }
    void parse(stack<NFA*> &operands) {
        exp(); // convert to reverse Polish notation
        construct(operands);
    }
};

/*
 * Convert the regular expression to NFA
 */
NFA* re_to_nfa(string re) {
    stack<NFA*> operands;
    parser(re).parse(operands);
    return operands.top();
}

void _span_epsilon(NFA nfa, set<int> states, set<int> &all_states, set<int> new_states) {
    for (trans &t : nfa.transitions) {
        std::set<int>::iterator it;
        for (it = states.begin(); it != states.end(); it++) {
            if (t.trans_symbol == '^' && t.vertex_from == *it) {
                new_states.insert(t.vertex_to);
                all_states.insert(t.vertex_to);
            }
        }
    }
    if (!new_states.empty()) {
        set<int> next;
        _span_epsilon(nfa, new_states, all_states, next);
    }
}

/*
 * Expand the state set with epsilon transitions
 */
void span_epsilon(NFA nfa, set<int> &states) {
    set<int> all_states = states;
    set<int> next;
    _span_epsilon(nfa, states, all_states, next);
    states.swap(all_states);
}

/*
 * Search the next possible states from each of states_in and put them in states_out
 */
void search_next_states(NFA nfa, char c, set<int> states_in, set<int> &states_out) {
    span_epsilon(nfa, states_in);
    for (trans &t : nfa.transitions) {
        std::set<int>::iterator it;
        for (it = states_in.begin(); it != states_in.end(); it++) {
            if (t.trans_symbol == c && t.vertex_from == *it) {
                states_out.insert(t.vertex_to);
            }
        }
    }
    span_epsilon(nfa, states_out);
}

/*
 * Check if the input is accepted by the NFA
 */
bool accepts(NFA nfa, string input) {
    set<int> states[input.length()];
    states[0] = {0};
    set<int> out;
    for (int i = 0; i < input.length(); i++) {
        if (i > 0) {
            states[i].swap(out); 
        }
        char c = input.at(i);       
        search_next_states(nfa, c, states[i], out);
    }
    set<int> last_states = out;
    return (find(last_states.begin(), last_states.end(), nfa.final_state) != last_states.end());
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("%s <regex> <input>\n", argv[0]);
        return -1;
    }
	NFA* required_nfa;
    std::string re(argv[1]);
	required_nfa = re_to_nfa(re);
	required_nfa->display();	

    std::string input(argv[2]);
    if (accepts(*required_nfa, input)) {
        cout<<"matched\n";
    } else {
        cout<<"does not match\n";
    }
	return 0;
}