#include <iostream>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <stack>
#include <cstdlib>
#include <stack>
#include <map>

using namespace std;

/**
 * Represents a position on the game grid with x and y coordinates
 */
struct Position {
    int x, y;

    Position(int x = 0, int y = 0) : x(x), y(y) {}

    // Comparison operators for use in sets and maps
    bool operator == (const Position &another) const {
        return x == another.x && y == another.y;
    }
    bool operator != (const Position &another) const {
        return !(*this == another);
    }
    bool operator < (const Position &another) const {
        if (x != another.x) return x < another.x;
        return y < another.y;
    }
};

/**
 * Represents an enemy entity with position and type token
 */
struct Enemy {
    Position position;
    char token;

    Enemy(Position pos, char t) : position(pos), token(t) {}

    bool operator < (const Enemy& other) const {
        if (position != other.position) return position < other.position;
        return token < other.token;
    }
};

/**
 * Generates Von Neumann neighborhood (4-directional) around a center position
 * @param center The center position for the neighborhood
 * @return Set of positions in the Von Neumann neighborhood
 */
set<Position> von_neumann_zone(Position &center) {
    set<Position> zone;
    if (center.x + 1 <= 12) zone.emplace(center.x + 1, center.y);
    if (center.x - 1 >= 0) zone.emplace(center.x - 1, center.y);
    if (center.y + 1 <= 12) zone.emplace(center.x, center.y + 1);
    if (center.y - 1 >= 0) zone.emplace(center.x, center.y - 1);
    return zone;
}

/**
 * Adds Von Neumann neighborhood positions to an existing zone set
 * @param center The center position for the neighborhood
 * @param initial_zone The set to which positions will be added
 */
void add_von_neumann_zone(Position &center, set<Position> &initial_zone) {
    if (center.x + 1 <= 12) initial_zone.emplace(center.x + 1, center.y);
    if (center.x - 1 >= 0) initial_zone.emplace(center.x - 1, center.y);
    if (center.y + 1 <= 12) initial_zone.emplace(center.x, center.y + 1);
    if (center.y - 1 >= 0) initial_zone.emplace(center.x, center.y - 1);
}

/**
 * Generates square (Moore) neighborhood around a center position
 * @param center The center position for the neighborhood
 * @param radius The radius of the square neighborhood
 * @return Set of positions in the square neighborhood
 */
set<Position> square_zone(Position &center, int radius) {
    set<Position> zone;
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int new_x = center.x + dx;
            int new_y = center.y + dy;

            if (new_x >= 0 && new_x <= 12 && new_y >= 0 && new_y <=12) {
                zone.emplace(new_x, new_y);
            }
        }
    }
    return zone;
}

/**
 * Adds square (Moore) neighborhood positions to an existing zone set
 * @param center The center position for the neighborhood
 * @param initial_zone The set to which positions will be added
 * @param radius The radius of the square neighborhood
 */
void add_square_zone(Position &center, set<Position> &initial_zone, int radius) {
    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            int new_x = center.x + dx;
            int new_y = center.y + dy;

            if (new_x >= 0 && new_x <= 12 && new_y >= 0 && new_y <=12) {
                initial_zone.emplace(new_x, new_y);
            }
        }
    }
}

/**
 * Generates double Von Neumann neighborhood (extended 4-directional)
 * @param center The center position for the neighborhood
 * @return Set of positions in the double Von Neumann neighborhood
 */
set<Position> double_von_neumann_zone(Position &center) {
    set<Position> zone = square_zone(center, 1);
    if (center.x + 2 <= 12) zone.emplace(center.x + 2, center.y);
    if (center.x - 2 >= 0) zone.emplace(center.x - 2, center.y);
    if (center.y + 2 <= 12) zone.emplace(center.x, center.y + 2);
    if (center.y - 2 >= 0) zone.emplace(center.x, center.y - 2);
    return zone;
}

/**
 * Abstract base class for Mithril coat states
 * Defines interface for handling dangerous zones based on coat state
 */
class MithrilCoatState {
public:
    virtual ~MithrilCoatState() = default;

    /**
     * Adds dangerous zones based on enemy type and current state
     * @param enemy_pos Position of the enemy
     * @param token Type of enemy
     * @param dangerous_zones_with_ring Set of dangerous zones when ring is worn
     */
    virtual void add_dangerous_zone(Position enemy_pos, char token, set<Position>& dangerous_zones_with_ring) = 0;

    /**
     * Checks if the coat is currently worn
     * @return True if coat is worn, false otherwise
     */
    virtual bool is_put_on() const = 0;

    /**
     * Gets the current dangerous zones
     * @return Reference to set of dangerous positions
     */
    const set<Position>& get_dangerous_zones() const { return dangerous_zones; }

    /**
     * Adds a specific position to dangerous zones
     * @param e_x X coordinate of dangerous position
     * @param e_y Y coordinate of dangerous position
     */
    void add_dangerous_zone(int e_x, int e_y) { dangerous_zones.emplace(e_x, e_y); }

    set<Position> dangerous_zones;  // Set of positions that are dangerous in current state
};

/**
 * Mithril coat state when coat is not worn
 * Implements base dangerous zone calculations
 */
class PutOff : public MithrilCoatState {
public:
    virtual bool is_put_on() const override {return false;}

    void add_dangerous_zone(Position enemy_pos, char token, set<Position>& dangerous_zones_with_ring) override {
        // Always add enemy's own position as dangerous
        dangerous_zones.insert(enemy_pos);
        dangerous_zones_with_ring.insert(enemy_pos);

        switch (token) {
            case 'P' : {
                // Perception zone - no additional dangerous zones
                break;
            }
            case 'O': {
                // Orc Patrol - Von Neumann neighborhood radius 1
                add_von_neumann_zone(enemy_pos, dangerous_zones);

                // Remove Von Neumann zone from ring-protected zones (Orcs can't see invisible Frodo)
                // With ring, only enemy cell is dangerous
                break;
            }
            case 'U': {
                // Uruk-hai - Von Neumann neighborhood radius 2
                Position right(enemy_pos.x + 1, enemy_pos.y);
                Position left(enemy_pos.x - 1, enemy_pos.y);
                Position up(enemy_pos.x, enemy_pos.y + 1);
                Position down(enemy_pos.x, enemy_pos.y - 1);

                add_von_neumann_zone(right, dangerous_zones);
                add_von_neumann_zone(left, dangerous_zones);
                add_von_neumann_zone(up, dangerous_zones);
                add_von_neumann_zone(down, dangerous_zones);

                // With ring, only immediate Von Neumann neighborhood remains dangerous
                add_von_neumann_zone(enemy_pos, dangerous_zones_with_ring);
                break;
            }
            case 'N': {
                // Nazgul - Moore neighborhood radius 1 with ears
                add_square_zone(enemy_pos, dangerous_zones, 1);

                // Add corner positions (ears)
                if (enemy_pos.x - 2 >= 0 && enemy_pos.y - 2 >= 0)
                    dangerous_zones.emplace(enemy_pos.x - 2, enemy_pos.y - 2);
                if (enemy_pos.x - 2 >= 0 && enemy_pos.y + 2 <= 12)
                    dangerous_zones.emplace(enemy_pos.x - 2, enemy_pos.y + 2);
                if (enemy_pos.x + 2 <= 12 && enemy_pos.y - 2 >= 0)
                    dangerous_zones.emplace(enemy_pos.x + 2, enemy_pos.y - 2);
                if (enemy_pos.x + 2 <= 12 && enemy_pos.y + 2 <= 12)
                    dangerous_zones.emplace(enemy_pos.x + 2, enemy_pos.y + 2);

                // With ring, expand to Moore neighborhood radius 2
                add_square_zone(enemy_pos, dangerous_zones_with_ring, 2);

                // Add extended ears for ring mode
                if (enemy_pos.x - 3 >= 0 && enemy_pos.y - 3 >= 0)
                    dangerous_zones_with_ring.emplace(enemy_pos.x - 3, enemy_pos.y - 3);
                if (enemy_pos.x - 3 >= 0 && enemy_pos.y + 3 <= 12)
                    dangerous_zones_with_ring.emplace(enemy_pos.x - 3, enemy_pos.y + 3);
                if (enemy_pos.x + 3 <= 12 && enemy_pos.y - 3 >= 0)
                    dangerous_zones_with_ring.emplace(enemy_pos.x + 3, enemy_pos.y - 3);
                if (enemy_pos.x + 3 <= 12 && enemy_pos.y + 3 <= 12)
                    dangerous_zones_with_ring.emplace(enemy_pos.x + 3, enemy_pos.y + 3);
                break;
            }
            case 'W': {
                // Watchtower - Moore neighborhood radius 2
                add_square_zone(enemy_pos, dangerous_zones, 2);
                add_square_zone(enemy_pos, dangerous_zones_with_ring, 2);

                // With ring, add extended corners
                if (enemy_pos.x - 3 >= 0 && enemy_pos.y - 3 >= 0)
                    dangerous_zones_with_ring.emplace(enemy_pos.x - 3, enemy_pos.y - 3);
                if (enemy_pos.x - 3 >= 0 && enemy_pos.y + 3 <= 12)
                    dangerous_zones_with_ring.emplace(enemy_pos.x - 3, enemy_pos.y + 3);
                if (enemy_pos.x + 3 <= 12 && enemy_pos.y - 3 >= 0)
                    dangerous_zones_with_ring.emplace(enemy_pos.x + 3, enemy_pos.y - 3);
                if (enemy_pos.x + 3 <= 12 && enemy_pos.y + 3 <= 12)
                    dangerous_zones_with_ring.emplace(enemy_pos.x + 3, enemy_pos.y + 3);
                break;
            }
            default : cerr << "Unknown enemy token: " << token << endl;
                break;
        }
    }
};

/**
 * Mithril coat state when coat is worn
 * Implements reduced dangerous zones for some enemies
 */
class PutOn : public MithrilCoatState {
public:
    virtual bool is_put_on() const override {return true;}

    void add_dangerous_zone(Position enemy_pos, char token, set<Position>& dangerous_zones_with_ring) override {
        // Always add enemy's own position as dangerous
        dangerous_zones.insert(enemy_pos);

        switch (token) {
            case 'P': {
                // Perception zone - no additional dangerous zones
                break;
            }
            case 'O': {
                // Orc Patrol - only enemy cell is dangerous with coat
                break;
            }
            case 'U': {
                // Uruk-hai - reduced to Von Neumann neighborhood radius 1
                add_von_neumann_zone(enemy_pos, dangerous_zones);
                break;
            }
            case 'N': {
                // Nazgul - Moore neighborhood radius 1 (no ears with coat)
                add_square_zone(enemy_pos, dangerous_zones, 1);
                break;
            }
            case 'W': {
                // Watchtower - unaffected by coat, Moore neighborhood radius 2 with extended corners
                add_square_zone(enemy_pos, dangerous_zones, 2);
                if (enemy_pos.x - 3 >= 0 && enemy_pos.y - 3 >= 0)
                    dangerous_zones.emplace(enemy_pos.x - 3, enemy_pos.y - 3);
                if (enemy_pos.x - 3 >= 0 && enemy_pos.y + 3 <= 12)
                    dangerous_zones.emplace(enemy_pos.x - 3, enemy_pos.y + 3);
                if (enemy_pos.x + 3 <= 12 && enemy_pos.y - 3 >= 0)
                    dangerous_zones.emplace(enemy_pos.x + 3, enemy_pos.y - 3);
                if (enemy_pos.x + 3 <= 12 && enemy_pos.y + 3 <= 12)
                    dangerous_zones.emplace(enemy_pos.x + 3, enemy_pos.y + 3);
                break;
            }
            default :
                cerr << "Unknown enemy token: " << token << endl;
                break;
        }
    }
};

/**
 * Represents the mental map of the game world
 * Tracks visited cells, enemies, dangerous zones, and supports backtracking
 */
class MindMap {
protected:
    unique_ptr<MithrilCoatState> coat_state = make_unique<PutOff>();  // Current coat state
    set<Position> visited_cells;              // Cells that have been visited
    set<Enemy> enemies;                       // Known enemies on the map
    set<Position> dangerous_zones_with_ring;  // Dangerous zones when ring is worn
    set<Position> unknown_dang_zones;         // Unknown dangerous zones

    // Backtracking data structures
    map<Position, Position> parent;           // Parent mapping for path reconstruction
    stack<Position> path_stack;               // Stack for backtracking algorithm

public:
    // Getters for various zone sets
    const set<Position>& get_visited() const {return visited_cells;}
    const set<Position>& get_dangerous_zones() const {return coat_state->get_dangerous_zones();}
    const set<Position>& get_dang_ring_zones() const {return dangerous_zones_with_ring;}

    /**
     * Adds an enemy to the mental map and calculates its dangerous zones
     * @param e_x X coordinate of enemy
     * @param e_y Y coordinate of enemy
     * @param token Type of enemy
     */
    void add_enemy(int e_x, int e_y, char &token) {
        enemies.emplace(Position(e_x, e_y), token);
        recalculate_zones();
    }

    /**
     * Recalculates all dangerous zones based on current enemies and state
     */
    void recalculate_zones() {
        coat_state->dangerous_zones.clear();
        dangerous_zones_with_ring.clear();
        for (auto& enemy : enemies) {
            coat_state->add_dangerous_zone(enemy.position, enemy.token, dangerous_zones_with_ring);
        }
    }

    /**
     * Switches to coat-worn state and recalculates dangerous zones
     */
    void put_on_coat() {
        coat_state = make_unique<PutOn>();
        recalculate_zones();
    }

    void clear_visited() {visited_cells.clear();}

    bool get_coat_state() {return coat_state->is_put_on();}
    void add_dangerous_zone(int e_x, int e_y) {coat_state->dangerous_zones.emplace(e_x, e_y);}
    void add_visited(Position &position) { visited_cells.insert(position);}
    void remove_visited(Position &position) { visited_cells.erase(position);}

    // Backtracking methods

    /**
     * Sets parent relationship for path reconstruction
     * @param child The child position
     * @param parent_pos The parent position
     */
    void set_parent(Position child, Position parent_pos) {
        parent[child] = parent_pos;
    }

    Position get_parent(Position pos) {
        return parent[pos];
    }

    /**
     * Pushes position to backtracking stack
     * @param pos Position to push to stack
     */
    void push_to_stack(Position pos) {
        path_stack.push(pos);
    }

    bool is_stack_empty() {
        return path_stack.empty();
    }

    /**
     * Pops position from backtracking stack
     * @return The top position from stack
     */
    Position pop_from_stack() {
        Position top = path_stack.top();
        path_stack.pop();
        return top;
    }

    void clear_stack() {
        while (!path_stack.empty()) {
            path_stack.pop();
        }
    }
};

/**
 * Main game controller class
 * Manages game state, movement, ring toggling, and backtracking algorithm
 */
class GameController {
private:
    enum Ring {ON, OFF};          // Ring state enumeration
    Ring ring = OFF;              // Current ring state
    MindMap mind_map;             // Mental map of the game world
    Position frodo_pos = Position(0, 0);  // Frodo's current position
    int step_count = 0;           // Step counter for limiting iterations
    int total_moves = 0;          // Actual moves across both search phases

public:
    GameController() = default;
    Position get_current_pos() const {return frodo_pos;}
    int get_total_moves() const {return total_moves;}
    void toggle_ring() { ring = (ring == ON) ? OFF : ON; }
    bool is_ring_on() const { return ring == ON; }
    void put_on_coat() {
        if (is_ring_on()) {
            toggle_ring();
            cout << "rr" << endl;
            cout.flush();
        }
        mind_map.put_on_coat();
    };
    void add_enemy(int e_x, int e_y, char &token) { mind_map.add_enemy(e_x, e_y, token); }

    /**
     * Processes game data from input
     * @param x X coordinate
     * @param y Y coordinate
     * @param token Entity type token
     */
    void load_data(int x, int y, char token) {
        if (token == 'C') put_on_coat();
        else if (token != 'G' && token != 'M' && token != 'R')  add_enemy(x, y, token);
    }

    /**
     * Resets internal state for new search phase
     * Clears visited cells and backtracking stack
     */
    void reset_for_new_phase() {
        mind_map.clear_visited();
        mind_map.clear_stack();
        step_count = 0;
    }

    /**
     * Checks if a move is valid without ring
     * @param mv Position to check
     * @return True if move is valid, false otherwise
     */
    bool is_valid_move_without_ring(Position mv) {
        return mv.x >= 0 && mv.x <= 12 && mv.y >= 0 && mv.y <= 12
               && mind_map.get_dangerous_zones().find(mv) == mind_map.get_dangerous_zones().end();
    }

    /**
     * Checks if a move is valid with ring
     * @param mv Position to check
     * @return True if move is valid, false otherwise
     */
    bool is_valid_move_with_ring(Position mv) {
        return mv.x >= 0 && mv.x <= 12 && mv.y >= 0 && mv.y <= 12
               && mind_map.get_dang_ring_zones().find(mv) == mind_map.get_dang_ring_zones().end();
    }

    /**
     * Checks if a move is valid considering current ring state
     * @param mv Position to check
     * @return True if move is valid, false otherwise
     */
    bool is_valid_move(Position mv) {
        if (is_ring_on()) {
            return is_valid_move_with_ring(mv);
        } else {
            return is_valid_move_without_ring(mv);
        }
    }

    /**
     * Automatically toggles ring state based on move safety
     * @param mv The intended move position
     */
    void ring_on(Position mv) {
        if (!mind_map.get_coat_state()) {
            bool valid_with_ring = is_valid_move_with_ring(mv);
            bool valid_without_ring = is_valid_move_without_ring(mv);

            // If current state is invalid but other state is valid, switch
            if (is_ring_on() && !valid_with_ring && valid_without_ring) {
                toggle_ring();
                cout << "rr" << endl;
                cout.flush();
            } else if (!is_ring_on() && !valid_without_ring && valid_with_ring) {
                toggle_ring();
                cout << "r" << endl;
                cout.flush();
            }
        }
    }

    /**
     * Executes one step of the backtracking algorithm towards target
     * @param target The target position to reach
     * @return True if step was successful, false if stuck or exceeded step limit
     */
    bool make_backtracking_step(Position target) {
        step_count++;
        if (step_count > 5000) {
            return false;
        }

        // Check if target reached
        if (frodo_pos == target) {
            return true;
        }

        // Mark current position as visited
        mind_map.add_visited(frodo_pos);

        // Generate possible moves
        vector<Position> poss_moves;
        poss_moves.emplace_back(frodo_pos.x, frodo_pos.y + 1); // right
        poss_moves.emplace_back(frodo_pos.x + 1, frodo_pos.y); // down
        poss_moves.emplace_back(frodo_pos.x, frodo_pos.y - 1); // left
        poss_moves.emplace_back(frodo_pos.x - 1, frodo_pos.y); // up

        // Priority 1: If target is adjacent, move to it
        for (auto &mv: poss_moves) {
            if (mv == target) {
                ring_on(mv);
                if (is_valid_move(mv)) {
                    frodo_pos = mv;
                    total_moves++;
                    cout << "m " << mv.x << " " << mv.y << endl;
                    cout.flush();
                    return true;
                }
            }
        }

        // Priority 2: Try unvisited valid cells
        for (auto &mv: poss_moves) {
            if (mind_map.get_visited().find(mv) == mind_map.get_visited().end()) {
                ring_on(mv);
                if (is_valid_move(mv)) {
                    mind_map.push_to_stack(frodo_pos);
                    frodo_pos = mv;
                    total_moves++;
                    cout << "m " << mv.x << " " << mv.y << endl;
                    cout.flush();
                    return true;
                }
            }
        }

        // Priority 3: Backtrack
        if (!mind_map.is_stack_empty()) {
            Position backtrack_pos = mind_map.pop_from_stack();
            ring_on(backtrack_pos);
            if (!is_valid_move(backtrack_pos)) return false;
            frodo_pos = backtrack_pos;
            total_moves++;
            cout << "m " << backtrack_pos.x << " " << backtrack_pos.y << endl;
            cout.flush();
            return true;
        }

        return false;
    }

};

/**
 * Main function - orchestrates the entire game flow
 * Handles input/output and coordinates the backtracking algorithm
 */
int main() {
    // Read initial game parameters
    int variant, gollum_x, gollum_y;
    cin >> variant >> gollum_x >> gollum_y;

    Position gollum_pos(gollum_x, gollum_y);
    GameController game;

    bool path_to_gollum_exists = true;
    int steps_since_progress = 0;
    const int MAX_STEPS_WITHOUT_PROGRESS = 1000;

    // Phase 1: Find Gollum
    while (game.get_current_pos() != gollum_pos && path_to_gollum_exists) {
        int num;
        cin >> num;

        // Read perception data
        for (int i=0; i < num; i++) {
            int x, y;
            char token;
            cin >> x >> y >> token;
            game.load_data(x, y, token);
        }

        Position old_pos = game.get_current_pos();
        bool step_success = game.make_backtracking_step(gollum_pos);

        if (!step_success) {
            path_to_gollum_exists = false;
            break;
        }

        // Track progress to detect stalls
        if (game.get_current_pos() == old_pos) {
            steps_since_progress++;
        } else {
            steps_since_progress = 0;
        }
        if (steps_since_progress > MAX_STEPS_WITHOUT_PROGRESS) {
            path_to_gollum_exists = false;
        }
    }

    if (!path_to_gollum_exists) {
        cout << "e -1" << endl;
        return 0;
    }

    int num;
    cin >> num;
    for (int i=0; i < num; i++) {
        int x, y;
        char token;
        cin >> x >> y >> token;
        game.load_data(x, y, token);
    }

    int md_x, md_y;
    cin >> md_x >> md_y;

    Position mount_doom_pos = Position(md_x, md_y);

    game.reset_for_new_phase();

    bool path_to_mount_doom_exists = true;
    steps_since_progress = 0;

    // Phase 2: Reach Mount Doom
    while (game.get_current_pos() != mount_doom_pos) {
        Position old_pos = game.get_current_pos();
        bool step_success = game.make_backtracking_step(mount_doom_pos);

        if (!step_success) {
            path_to_mount_doom_exists = false;
            break;
        }

        // Track progress to detect stalls
        if (game.get_current_pos() == old_pos) {
            steps_since_progress++;
        } else {
            steps_since_progress = 0;
        }

        if (steps_since_progress > MAX_STEPS_WITHOUT_PROGRESS) {
            path_to_mount_doom_exists = false;
            break;
        }

        cin >> num;
        for (int i=0; i < num; i++) {
            int x, y;
            char token;
            cin >> x >> y >> token;
            game.load_data(x, y, token);
        }
    }

    // Output final result
    if (!path_to_mount_doom_exists) {
        cout << "e -1" << endl;
    } else {
        cout << "e " << game.get_total_moves() << endl;
    }

    return 0;
} //
