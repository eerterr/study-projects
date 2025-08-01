#include <iostream>
using namespace std;
 
class FST {
private:
    string sub;
    int state;
public:
    static int counter;
 
    FST (string str) : sub(str), state(0){}
 
    int process (char c) {
        state = transition_function(state, c, sub);
        return output_function(state);
    }
    int transition_function(int state, char c, string substring){
        if (state == substring.length()) {
            counter++;
            if (c == substring[0]) return 1;
            else return 0;
        }
        if (c == substring[state]) return ++state;
        else {
            if (c == substring[0]) return 1;
            else return 0;
        }
    }
    int output_function(int current_state){
        return current_state != 0 ? 1 : 0;
    }
    int get_state() {return state;}
};
 
int FST::counter = 0;
 
int main() {
    string C, S;
    cin>>C>>S;
 
    FST fst(C);
    for (char c : S) {
        fst.process(c);
    }
    if (fst.get_state() == C.length()) {
        FST::counter++;
    }
    if (FST::counter == 0) cout<<"NOT DETECTED";
    else cout<<FST::counter;
 
    return 0;
}
close
