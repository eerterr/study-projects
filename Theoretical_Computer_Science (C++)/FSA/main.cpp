#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <regex>
#include <string>
#include <set>

using namespace std;

enum Type {DETERMINISTIC, NON_DETERMINISTIC};
class Transition;

class State {
public:
    string name;

    vector<Transition*> outgoing;
    State(string name) : name(name) {}
};

class Transition {
public:
    shared_ptr<State> from;
    shared_ptr<State> to;
    string condition;
    Transition(shared_ptr<State> from, shared_ptr<State> to, string condition) : from(from), to(to), condition(condition) {}
};
vector<string> to_array(const string& input) {
    int start_index;
    int n = input.length();
    for (int i = 0; i < n; i++) {
        if (input[i] == '[') start_index = i+1;
    }
    int last_index = n-2;

    string content = input.substr(start_index, last_index - start_index  + 1);

    vector<string> result;
    stringstream ss(content);
    string token;

    while (getline(ss, token, ',')) {
        result.push_back(token);
    }
    return result;
}

string brackets(string& str) {
    return "(" + str + ")";
}
struct state_cmp {
    bool operator()(const shared_ptr<State>& x, const shared_ptr<State>& y) const {
        return x->name < y->name;
    }
};

void remove_duplicates_state(vector<shared_ptr<State>>& states) {
    set<shared_ptr<State>, state_cmp> unique_elements(states.begin(), states.end());
    states.assign(unique_elements.begin(), unique_elements.end());
}
void remove_duplicates_string(vector<string>& strings) {
    set<string> unique_elements(strings.begin(), strings.end());
    strings.assign(unique_elements.begin(), unique_elements.end());
}
bool valid_format(const string& str) {
    regex pattern(R"(^\w+=\[(.*?)\]$)");
    return regex_match(str, pattern);
}
bool in_transitions(string& from, string& condition, string& to, vector<pair<string, pair<string, string>>> transitions) {
    for (auto transition : transitions) {
        if (transition.second.first == from && transition.first == condition && transition.second.second == to){
            return true;
        }
    }
    return false;
}

class FSA {
private:
    Type type;
    unordered_map<string, shared_ptr<State>> state_map;
    vector<shared_ptr<State>> states;
    vector<string> alphabet;
    vector<shared_ptr<State>> accepting;
    shared_ptr<State> initial;
    vector<Transition*> transitions;

    static bool cmp(const shared_ptr<State> current, const shared_ptr<State> next) {
        return current->name < next->name;
    }

public:

    FSA (string& type_str, vector<string>& states_vec, vector<string>& alphabet_vec, string& initial_str, vector<string>& accepting_vec, vector<pair<string, pair<string, string>>>&  transitions_vec) {

        type = (type_str == "deterministic") ? DETERMINISTIC : NON_DETERMINISTIC;

        for (auto state_str : states_vec) {
            states.emplace_back(add_state(state_str));
        }
        remove_duplicates_state(states);
        sort(states.begin(), states.end(), cmp);

        for (auto symbol : alphabet_vec) {
            alphabet.emplace_back(symbol);
        }
        remove_duplicates_string(alphabet);

        initial = find_state(initial_str);

        for (auto accepting_state_str : accepting_vec) {
            accepting.emplace_back(find_state(accepting_state_str));
        }
        remove_duplicates_state(accepting);
        sort(accepting.begin(), accepting.end(), cmp);

        for (auto t : transitions_vec) {
            add_transition(t.second.first, t.second.second, t.first);
        }
    }
    shared_ptr<State> add_state(const string& name) {
        if (state_map.count(name)) {
            return state_map[name];
        }
        auto s = make_shared<State>(name);
        state_map[name] = s;
        states.push_back(s);
        return s;
    }


    Transition* add_transition(string x, string y, string condition) {
        shared_ptr<State> from = find_state(x);
        shared_ptr<State> to = find_state(y);

        for (auto transition : transitions) {
            if (transition->from == from
                && transition->to == to
                && transition->condition == condition) return transition;
        }

        auto* transition = new Transition(from, to, condition);

        transitions.push_back(transition);
        from->outgoing.emplace_back(transition);
        to->outgoing.emplace_back(transition);

        return transition;
    }

    shared_ptr<State> find_state(string name) {
        for (auto s : states) {
            if (s->name == name) return s;
        }
        return nullptr;
    }

    string find_in_alphabet(string symbol) {
        for (auto a : alphabet) {
            if (a == symbol) return a;
        }
        return "";
    }
    void kleene() {
        int n = states.size();
        vector<vector<vector<string>>> R(n + 1, vector<vector<string>>(n, vector<string>(n, "{}")));

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                string value = "";
                for (auto t : transitions) {
                    if (t->from == states[i] && t->to == states[j]) {
                        if (!value.empty()) value += "|";
                        value += t->condition;
                    }
                }
                if (i == j) {
                    if (!value.empty()) value += "|eps";
                    else value = "eps";
                }
                if (!value.empty()) R[0][i][j] = value;
            }
        }

        for (int k = 1; k <= n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    string expression =
                            brackets(R[k - 1][i][k - 1])
                            + brackets(R[k - 1][k - 1][k - 1]) + "*"
                            + brackets(R[k - 1][k - 1][j]) +  "|"
                            + brackets(R[k - 1][i][j]);

                    R[k][i][j] = expression;
                }
            }
        }

        int start = find(states.begin(), states.end(), initial) - states.begin();
        string answer = "";

        for (shared_ptr<State> accept : accepting) {
            int end = find(states.begin(), states.end(), accept) - states.begin();
            if (!answer.empty()) answer += "|";
            answer += brackets(R[n][start][end]);
        }
        cout<<answer;
    }

    int condition_in_state(string& c, shared_ptr<State> state) {
        int repeats = 0;
        for (auto transition : transitions) {
            if (transition->from == state && transition->condition == c) {
                repeats++;
            }
        }
        return repeats;
    }

    bool is_connected() {
        unordered_set<shared_ptr<State>> in_queue;
        vector<shared_ptr<State>> queue;

        in_queue.insert(initial);
        queue.push_back(initial);

        int n = queue.size() - 1;
        while (!queue.empty()) {
            shared_ptr<State> s = queue[n];
            queue.erase(queue.begin() + n);
            for (Transition* transition : s->outgoing) {
                if (transition->from != s) continue;

                if (in_queue.count(transition->to) == 0) {
                    in_queue.insert(transition->to);
                    queue.push_back(transition->to);
                }
            }
        }
        return states.size() == in_queue.size();
    }

    bool is_deterministic() {
        bool flag = false;
        if (!find_in_alphabet("eps").empty()) flag = true;
        if (!flag) {
            for (auto state: states) {
                for (auto transition: state->outgoing) {
                    if (condition_in_state(transition->condition, state) > 1) {
                        flag = true;
                        break;
                    }
                }
            }
        }
        if (!flag) {
            for (auto state : states) {
                for (auto transition : state->outgoing) {
                    if (transition->condition == "eps") {
                        flag = true;
                        break;
                    }
                }
            }
        }
        if (type == DETERMINISTIC && flag) {
            return false;
        }
        return true;
    }

    ~FSA() {
        for (auto t : transitions) delete t;
        states.clear();
        transitions.clear();
    }
};
string parse_key(const string& input) {
    int start_index = 0;
    int last_index;
    int n = input.length();
    for (int i = 0; i < n; i++) {
        if (input[i] == '=') last_index = i-1;
    }
    string key = input.substr(start_index, last_index - start_index  + 1);
    return key;
}
int main() {
    vector<string> keys = {"type", "states", "alphabet", "initial", "accepting", "transitions"};
    string type_str, states_str, alphabet_str, initial_str, accepting_str, transitions_str;
    vector<string> states_vec; vector<string> alphabet_vec; vector<string> accepting_vec; vector<pair<string, pair<string, string>>> transitions_vec;

    vector<string> lines;
    string line;
    while (getline(cin, line)) {
        if (line.empty()) break;
        lines.push_back(line);
    }
    for (int i = 0; i < min(keys.size(), lines.size()); i++) {
        string key = parse_key(lines[i]);
        if (find(keys.begin(), keys.end(), key) == keys.end()) {
            continue;
        }
        if (key != keys[i]) {
            cout << "Input is malformed." << endl;
            return 0;
        }
    }

    string type_value, initial_value;

    for (const string& current : lines) {
        string key = parse_key(current);
        if (key == "type") {
            type_str = current;
            if (type_str.empty() || !valid_format(type_str) || type_str.find("type") == string::npos) {
                cout << "Input is malformed." << endl;
                return 0;
            }
            size_t start_for_type = type_str.find('[') + 1;
            size_t end_for_type = type_str.find(']');

            type_value = type_str.substr(start_for_type, end_for_type - start_for_type);

            if (type_value != "deterministic" && type_value != "non-deterministic") {
                cout << "Input is malformed." << endl;
                return 0;
            }
        }
        else if (key == "states") {
            states_str = current;
            if (states_str.empty() || !valid_format(states_str) || states_str.find(",,") != string::npos ||
                states_str.find("states") == string::npos) {
                cout << "Input is malformed." << endl;
                return 0;
            }
            for (const string &state: to_array(states_str)) {
                states_vec.emplace_back(state);
            }
            if (states_vec.empty()) {
                cout << "Input is malformed." << endl;
                return 0;
            }
            if (states_vec.back()[states_vec.back().length() - 1] == ',') {
                cout << "Input is malformed." << endl;
                return 0;
            }
        }
        else if (key == "alphabet") {
            alphabet_str = current;
            if (alphabet_str.empty() || !valid_format(alphabet_str) || alphabet_str.find(",,") != string::npos ||
                alphabet_str.find("alphabet") == string::npos) {
                cout << "Input is malformed." << endl;
                return 0;
            }
            for (const string &item: to_array(alphabet_str)) {
                alphabet_vec.emplace_back(item);
            }
            if (alphabet_vec.empty()) {
                cout << "Input is malformed." << endl;
                return 0;
            }
        }
        else if (key == "initial") {
            initial_str = current;
            size_t start_for_initial = initial_str.find('[') + 1;
            size_t end_for_initial = initial_str.find(']');

            initial_value = initial_str.substr(start_for_initial, end_for_initial - start_for_initial);

            if (initial_value.empty() || initial_str.find("initial") == string::npos) {
                cout << "Initial state is not defined.";
                return 0;
            }
            if (find(states_vec.begin(), states_vec.end(), initial_value) == states_vec.end()) {
                cout << "A state '" + initial_value + "' is not in the set of states.";
                return 0;
            }
            if (initial_value[initial_value.length() - 1] == ',') {
                cout << "Input is malformed." << endl;
                return 0;
            }
        }
        else if (key == "accepting") {
            accepting_str = current;
            if (!valid_format(accepting_str) || accepting_str.find(",,") != string::npos ||
                accepting_str.find("accepting") == string::npos) {
                cout << "Input is malformed." << endl;
                return 0;
            }

            for (const string &accepting_state: to_array(accepting_str)) {
                if (find(states_vec.begin(), states_vec.end(), accepting_state) == states_vec.end()) {
                    cout << "A state '" + accepting_state + "' is not in the set of states.";
                    return 0;
                }
                accepting_vec.emplace_back(accepting_state);
            }
            if (accepting_vec.empty()) {
                cout << "Set of accepting states is empty.";
                return 0;
            }
            if (accepting_vec.back()[accepting_vec.back().length() - 1] == ',') {
                cout << "Input is malformed." << endl;
                return 0;
            }
        }
        else if (key == "transitions") {
            transitions_str = current;
            if (transitions_str.empty() || !valid_format(transitions_str)
                || transitions_str.find(",,") != string::npos
                || transitions_str.find(">>") != string::npos || transitions_str.find("transitions") == string::npos) {
                cout << "Input is malformed." << endl;
                return 0;
            }
            if (transitions_str[transitions_str.length() - 2] == ',') {
                cout << "Input is malformed." << endl;
                return 0;
            }
            for (const string &transition: to_array(transitions_str)) {
                vector<string> tokens;
                stringstream ss(transition);
                string token;
                while (getline(ss, token, '>')) tokens.push_back(token);
                if (tokens.size() != 3 || in_transitions(tokens[0], tokens[1], tokens[2], transitions_vec)) {
                    cout << "Input is malformed." << endl;
                    return 0;
                }

                if (find(states_vec.begin(), states_vec.end(), tokens[0]) == states_vec.end()) {
                    cout << "A state '" + tokens[0] + "' is not in the set of states.";
                    return 0;
                }
                if (find(states_vec.begin(), states_vec.end(), tokens[2]) == states_vec.end()) {
                    cout << "A state '" + tokens[2] + "' is not in the set of states.";
                    return 0;
                }
                if (find(alphabet_vec.begin(), alphabet_vec.end(), tokens[1]) == alphabet_vec.end()) {
                    cout << "A transition symbol '" + tokens[1] + "' is not in the alphabet.";
                    return 0;
                }
                transitions_vec.emplace_back(make_pair(tokens[1], make_pair(tokens[0], tokens[2])));
            }
        } else {
            cout << "Input is malformed." << endl;
            return 0;
        }
    }

    FSA fsa(type_value, states_vec, alphabet_vec, initial_value, accepting_vec, transitions_vec);

    if (!fsa.is_deterministic()) {
        cout<<"FSA is non-deterministic.";
    }
    else if (!fsa.is_connected()) {
        cout<<"Some states are disjoint.";
    }
    else fsa.kleene();

    return 0;
}
