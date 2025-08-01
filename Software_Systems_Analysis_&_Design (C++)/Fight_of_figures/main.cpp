#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

/**
 * Represents a coin with a specific value,
 * uses Flyweight pattern through CoinFactory to share identical coin values
 */
class Coin {
private:
    int value;
public:
    //parameterized constructor
    Coin(int value) : value(value) {}
    //getting coin's value
    int get_value() const { return value; }

    ~Coin() = default;
};
/**
 * Factory class for creating and caching Coin objects to prevent duplicates,
 * implements Flyweight pattern for memory optimization
 */
class CoinFactory {
private:
    //cache to store coins by values
    static unordered_map<int, shared_ptr<Coin>> cache;
public:
    //returns a shared_ptr to a Coin with the given value, creating it if necessary
    static shared_ptr<Coin> get_coin(int value) {
        auto x = cache.find(value);
        if (x == cache.end()) {
            x = cache.emplace(value, make_shared<Coin>(value)).first;
        }
        return x->second;
    }
    ~CoinFactory () = default;
};

unordered_map<int, shared_ptr<Coin>> CoinFactory::cache;

/**
 * Represents coin position on board and its collection state,
 * uses shared_ptr from CoinFactory to reference coin value
 */
class CoinCoordinates {
public:
    //coordinates of the coin
    int x;
    int y;

    shared_ptr<Coin> coin;

    //parameterized constructor
    CoinCoordinates(int x, int y, int value) : x(x), y(y), coin(CoinFactory::get_coin(value)) {}

    //marks the coin as collected by resetting the shared_ptr
    void collect() {
        if (coin) {
            coin.reset();
        }
    }
    //checks if the coin has been collected
    bool collected() const { return coin == nullptr; }

    //returns the value of the coin if it exists
    int get_value() const { return coin ? coin->get_value() : 0; }
};

/**
 * Abstract base class for figure movement states,
 * implements State pattern for behavior change
 */
class State {
public:
    //returns the step size depend on state
    virtual int step() const = 0;
    //returns the next state after style change
    virtual unique_ptr<State> style() const = 0;
    virtual ~State() = default;
};

/**
 * Class for Normal state
 * (step = 1, next state - attacking)
 */
class NormalMode : public State {
public:
    int step() const override { return 1; }
    unique_ptr<State> style() const override;
};

/**
 * Class for Attacking state
 * (step = 2, next state - normal)
 */
class AttackingMode : public State {
public:
    int step() const override { return 2; }
    unique_ptr<State> style() const override {
        cout << " CHANGED STYLE TO NORMAL" << endl;
        return make_unique<NormalMode>();
    }
};
//late initialization of style function for the normal mode
unique_ptr<State> NormalMode::style() const {
    cout << " CHANGED STYLE TO ATTACKING" << endl;
    return make_unique<AttackingMode>();
}

/**
 * Prototype interface for cloning figures.
 */
template <typename Derived>
class FigurePrototype {
public:
    virtual unique_ptr<Derived> clone() const = 0;
    virtual ~FigurePrototype() {};
};

/**
 * Game figure with position, movement state and cloning capabilities
 * Combines Prototype pattern for cloning and State pattern for movement
 */
class Figure : FigurePrototype<Figure> {
public:
    //figure's coordinates
    int x, y;
    //figure mode (Normal / Attacking)
    unique_ptr<State> mode;

    bool is_clone = false;
    bool has_clone = false;
    bool is_alive = true;

    //default constructor
    Figure() : x(0), y(0), mode(make_unique<NormalMode>()) {}
    //parameterized constructor
    Figure(int x_value, int y_value) : x(x_value), y(y_value), mode(make_unique<NormalMode>()) {}
    //copy constructor
    Figure(const Figure& other) : x(other.y), y(other.x), mode(make_unique<NormalMode>()), is_clone(true), is_alive(true), has_clone(false) {}
    //rewrite "=" operator
    Figure& operator=(const Figure& other) {
        if (this != &other) {
            x = other.x;
            y = other.y;
            is_clone = other.is_clone;
            has_clone = other.has_clone;
            is_alive = other.is_alive;
            mode = make_unique<NormalMode>();
        }
        return *this;
    }
    //checking if the figure can be cloned
    bool can_clone(int n, const vector<CoinCoordinates>& coins, const pair<Figure, Figure>& team, const pair<Figure, Figure>& enemy_team) const {
        //if figure is already cloned
        if (has_clone) return false;
        //if it's dead or is dead or coordinates are equal
        if (!is_alive || is_clone || x == y) return false;

        int clone_x = y;
        int clone_y = x;

        //check if the target position is occupied by any coin
        for (const auto& coin : coins) {
            if (coin.x == clone_x && coin.y == clone_y && !coin.collected()) return false;
        }
        //check if the target position is occupied by any figure
        if ((team.first.is_alive && team.first.x == y && team.first.y == x) ||
            (team.second.is_alive && team.second.x == y && team.second.y == x) ||
            (enemy_team.first.is_alive && enemy_team.first.x == y && enemy_team.first.y == x) ||
            (enemy_team.second.is_alive && enemy_team.second.x == y && enemy_team.second.y == x)) {
            return false;
        }
        return true;
    }

    //cloning the figure according to the game rules (Prototype pattern)
    unique_ptr<Figure> clone() const override {
        auto new_clone = make_unique<Figure>(*this);
        new_clone->is_clone = true;
        new_clone->has_clone = false;
        return new_clone;
    }

    //changes the figure's style (State pattern)
    void style() {
        mode = mode->style();
    }

    ~Figure() = default;
};
//common function for figures moving
string move_figure(Figure& figure, string direction, int n, vector<CoinCoordinates>& coins, pair<Figure, Figure>& same_team, pair<Figure, Figure>& enemy_team, int& team_score, const char& team_name) {
    //get step size depend on mode of the figure
    int step_size = figure.mode->step();
    //set initial state for new coordinates for their future upgrading
    int new_x = figure.x, new_y = figure.y;
    //change coordinates values according to the direction and step's size
    if (direction == "UP") new_x -= step_size;
    else if (direction == "RIGHT") new_y += step_size;
    else if (direction == "DOWN") new_x += step_size;
    else if (direction == "LEFT") new_y -= step_size;
    else {
        cout << "INVALID ACTION" << endl;
        return "";
    }
    //check if figure is dead and can't move
    if (!figure.is_alive) {
        cout << "INVALID ACTION" << endl;
        return "";
    }
    //check is new coordinates are possible
    if (new_x < 1 || new_x > n || new_y < 1 || new_y > n) {
        cout << "INVALID ACTION" << endl;
        return "";
    }
    //check if cell is already occupated by the same team member
    if ((same_team.first.is_alive && same_team.first.x == new_x && same_team.first.y == new_y) ||
        (same_team.second.is_alive && same_team.second.x == new_x && same_team.second.y == new_y)) {
        cout << "INVALID ACTION" << endl;
        return "";
    }

    figure.x = new_x;
    figure.y = new_y;
    //define how many points figure get after moving
    int score_increasing = 0;
    //looking for coin with the same coordinates and then collect it if it's exist
    for (auto& coin : coins) {
        if (!coin.collected() && coin.x == new_x && coin.y == new_y) {
            score_increasing = coin.get_value();
            team_score += score_increasing;
            coin.collect();
            break;
        }
    }
    //define message of killing some figure if it's possible
    string kill_message = "";
    //check if cell is occupated by another team member and kill it if cell is occupated
    switch (team_name) {
        //for green command
        case 'G': {
            //if there is some original member of the opposite team
            if (enemy_team.first.is_alive && enemy_team.first.x == new_x && enemy_team.first.y == new_y) {
                enemy_team.first.is_alive = false;
                kill_message = " AND KILLED RED";
            }
            //if there is some clone member of the opposite team
            else if (enemy_team.second.is_alive && enemy_team.second.x == new_x && enemy_team.second.y == new_y) {
                enemy_team.second.is_alive = false;
                kill_message = " AND KILLED REDCLONE";
            }
            break;
        }
        //for red command
        case 'R': {
            //if there is some original member of the opposite team
            if (enemy_team.first.is_alive && enemy_team.first.x == new_x && enemy_team.first.y == new_y) {
                enemy_team.first.is_alive = false;
                kill_message = " AND KILLED GREEN";
            }
            //if there is some clone member of the opposite team
            else if (enemy_team.second.is_alive && enemy_team.second.x == new_x && enemy_team.second.y == new_y) {
                enemy_team.second.is_alive = false;
                kill_message = " AND KILLED GREENCLONE";
            }
            break;
        }
    }
    //depends on changing (score, killed figure or just only movement) print some message
    if (score_increasing != 0) {
        return to_string(new_x) + " " + to_string(new_y) + " AND COLLECTED " + to_string(score_increasing);
    }
    else if (!kill_message.empty()) {
        return to_string(new_x) + " " + to_string(new_y) + kill_message;
    }
    else return to_string(new_x) + " " + to_string(new_y);
}

int main() {
    //read square plane's side size
    int n;
    cin >> n;
    cin.ignore();

    //define pair (original and clone) for the green team...
    pair<Figure, Figure> green_team;
    //...and score for this command
    int green_score = 0;

    //define pair (original and clone) for the red team...
    pair<Figure, Figure> red_team;
    //...and score for this command
    int red_score = 0;

    //read start position for the original green figure
    int start_x_green, start_y_green;
    cin >> start_x_green >> start_y_green;
    green_team.first = Figure(start_x_green, start_y_green);

    //read start position for the original red figure
    int start_x_red, start_y_red;
    cin >> start_x_red >> start_y_red;
    red_team.first = Figure(start_x_red, start_y_red);

    //define vector with coordinates of all coins
    vector<CoinCoordinates> coins;

    //read number of coins
    int m;
    cin >> m;
    cin.ignore();

    //read values and coordinates for all coins
    int temp_x, temp_y, temp_value;
    for (int i = 0; i < m; i++) {
        cin >> temp_x >> temp_y >> temp_value;
        coins.emplace_back(CoinCoordinates(temp_x, temp_y, temp_value));
    }

    //read number of commands
    int p;
    cin >> p;
    cin.ignore();

    //read all p commands
    for (int i = 0; i < p; i++) {
        string line;
        getline(cin, line);
        istringstream stream(line);
        vector<string> command;
        string token;
        while (stream >> token) command.push_back(token);

        if (command[1] == "COPY") {
            if (command[0] == "GREEN") {
                //check is it possible to make the green clone...
                if (green_team.first.can_clone(n, coins, green_team, red_team)) {
                    //...and make it if it's possible
                    auto clone_ptr = green_team.first.clone();
                    green_team.first.has_clone = true;
                    green_team.second = *clone_ptr;
                    cout << "GREEN CLONED TO " << green_team.second.x << " " << green_team.second.y << endl;
                }
                //if it's impossible, print error message
                else {
                    cout << "INVALID ACTION" << endl;
                }
            }
            else if (command[0] == "RED") {
                //check is it possible to make the red clone...
                if (red_team.first.can_clone(n, coins, red_team, green_team)) {
                    //...and make it if it's possible
                    auto clone_ptr = red_team.first.clone();
                    red_team.first.has_clone = true;
                    red_team.second = *clone_ptr;
                    cout << "RED CLONED TO " << red_team.second.x << " " << red_team.second.y << endl;
                }
                    //if it's impossible, print error message
                else {
                    cout << "INVALID ACTION" << endl;
                }
            }
            //if it's not green and not red print error message
            else {
                cout << "INVALID ACTION" << endl;
            }
        }
        else if (command[1] == "STYLE") {
            if (command[0] == "GREEN") {
                //check is original green figure alive
                if (!green_team.first.is_alive) {
                    //if not, print error message
                    cout << "INVALID ACTION" << endl;
                } else {
                    //if alive - style
                    cout << command[0];
                    green_team.first.style();
                }
            }
            else if (command[0] == "RED") {
                //check is original red figure alive
                if (!red_team.first.is_alive) {
                    //if not, print error message
                    cout << "INVALID ACTION" << endl;
                } else {
                    //if alive - style
                    cout << command[0];
                    red_team.first.style();
                }
            }
            else if (command[0] == "GREENCLONE") {
                //check is clone green figure alive and exist
                if (green_team.second.is_alive && green_team.first.has_clone) {
                    //if alive - style
                    cout << command[0];
                    green_team.second.style();
                }
                //if not, print error message
                else cout << "INVALID ACTION" << endl;
            }
            else if (command[0] == "REDCLONE") {
                //check is clone red figure alive and exist
                if (red_team.second.is_alive && red_team.first.has_clone) {
                    //if alive - style
                    cout << command[0];
                    red_team.second.style();
                }
                //if not, print error message
                else cout << "INVALID ACTION" << endl;
            }
        }
        else { // MOVE
            string coordinates;

            //depend on figure call common move_figure function with different arguments
            if (command[0] == "GREEN") {
                coordinates = move_figure(green_team.first, command[1], n, coins, green_team, red_team, green_score, 'G');
            }
            else if (command[0] == "RED") {
                coordinates = move_figure(red_team.first, command[1], n, coins, red_team, green_team, red_score, 'R');
            }
            else if (command[0] == "GREENCLONE") {
                if (green_team.second.is_alive)
                    coordinates = move_figure(green_team.second, command[1], n, coins, green_team, red_team, green_score, 'G');
                else {
                    cout << "INVALID ACTION" << endl;
                    continue;
                }
            }
            else if (command[0] == "REDCLONE") {
                if (red_team.second.is_alive)
                    coordinates = move_figure(red_team.second, command[1], n, coins, red_team, green_team, red_score, 'R');
                else {
                    cout << "INVALID ACTION" << endl;
                    continue;
                }
            }
            //check if movement are made successful
            if (!coordinates.empty()) {
                cout << command[0] << " MOVED TO " << coordinates << endl;
            }
        }
    }

    //depend on how green team score related to the red team score, print message about winner (or tie)
    if (green_score > red_score) {
        cout << "GREEN TEAM WINS. SCORE " << green_score << " " << red_score;
    } else if (green_score < red_score) {
        cout << "RED TEAM WINS. SCORE " << green_score << " " << red_score;
    } else {
        cout << "TIE. SCORE " << green_score << " " << red_score;
    }

    return 0;
}
