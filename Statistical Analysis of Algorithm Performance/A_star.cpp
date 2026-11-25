#include <iostream>
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <queue>
#include <functional>
#include <cstdlib>

using namespace std;

/**
 * Represents a position on the game grid with x and y coordinates
 */
struct Position {
    int x, y;

    Position(int x = 0, int y = 0) : x(x), y(y) {}

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
 * A search state includes the ring because the same cell can have different safe exits
 */
struct SearchState {
    Position position;
    bool ring_on;

    bool operator == (const SearchState &another) const {
        return position == another.position && ring_on == another.ring_on;
    }
    bool operator < (const SearchState &another) const {
        if (position != another.position) return position < another.position;
        return ring_on < another.ring_on;
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
set<Position> square_zone(Position &center, int radious) {
    set<Position> zone;
    for (int dx = -radious; dx <= radious; dx++) {
        for (int dy = -radious; dy <= radious; dy++) {
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
void add_square_zone(Position &center, set<Position> &initial_zone, int radious) {
    for (int dx = -radious; dx <= radious; dx++) {
        for (int dy = -radious; dy <= radious; dy++) {
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

    set<Position> dangerous_zones; // Set of positions that are dangerous in current state
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
 * Implements reduced dangerous zones of some enemies
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
    unique_ptr<MithrilCoatState> coat_state = make_unique<PutOff>(); // Current coat state

    set<Position> visited_cells;              // Cells that have been visited
    set<Enemy> enemies;                       // Known enemies on the map
    set<Position> dangerous_zones_with_ring;  // Dangerous zones when ring is worn
    set<Position> unknown_dang_zones;         // Unknown dangerous zones

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

    bool get_coat_state() {return coat_state->is_put_on();}
    void add_dangerous_zone(int e_x, int e_y) {coat_state->dangerous_zones.emplace(e_x, e_y);}
    void add_visited(Position &position) { visited_cells.insert(position);}
    void clear_visited() { visited_cells.clear(); }
};

/**
 * Main game controller class
 * Manages game state, movement, ring toggling, and backtracking algorithm
 */
class GameController {
private:
    enum Ring {ON, OFF}; // Ring state enumeration

protected:
    Ring ring = OFF;                            // Current ring state
    MindMap mind_map;                           // Mental map of the game world
    Position frodo_pos = Position(0, 0);  // Frodo's current position
    int step_count = 0;                         // Step counter (non-static now)
    int total_moves = 0;                        // Actual moves across both search phases

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
     * Resets state for new search phase
     */
    void reset_for_new_phase() {
        mind_map.clear_visited();
        step_count = 0;
    }

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
     * Checks if a move is valid considering current state and dangerous zones
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

    bool is_valid_move_with_ring(Position mv) {
        return mv.x >= 0 && mv.x <= 12 && mv.y >= 0 && mv.y <= 12
               && mind_map.get_dang_ring_zones().find(mv) == mind_map.get_dang_ring_zones().end();
    }

    bool is_valid_move_without_ring(Position mv) {
        return mv.x >= 0 && mv.x <= 12 && mv.y >= 0 && mv.y <= 12
               && mind_map.get_dangerous_zones().find(mv) == mind_map.get_dangerous_zones().end();
    }

    /**
     * Automatically toggles ring state based on move safety
     * @param mv The intended move position
     */
    void ring_on(Position mv) {
        if (!mind_map.get_coat_state()) {
            bool valid_with_ring = is_valid_move_with_ring(mv);
            bool valid_without_ring = is_valid_move_without_ring(mv);

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
     * Executes one step of the A* algorithm towards target
     * Uses Manhattan distance as heuristic and considers both ring states
     * @param target represent our goal
     */
    bool make_astar_step(Position target) {
        step_count++;
        if (step_count > 5000) return false;
        if (frodo_pos == target) return true;

        // Replan after each observation: unknown cells are treated as safe until discovered.
        auto heuristic = [&](Position pos) {
            return abs(pos.x - target.x) + abs(pos.y - target.y);
        };

        using QueueEntry = pair<int, SearchState>;
        priority_queue<QueueEntry, vector<QueueEntry>, greater<QueueEntry>> open;
        map<SearchState, int> g_score;
        map<SearchState, SearchState> parent;

        SearchState start{frodo_pos, is_ring_on()};
        SearchState goal;
        bool found = false;
        g_score[start] = 0;
        open.push({heuristic(frodo_pos), start});

        while (!open.empty()) {
            QueueEntry entry = open.top();
            open.pop();
            SearchState current = entry.second;
            int current_g = g_score[current];
            if (entry.first != current_g + heuristic(current.position)) continue;

            if (current.position == target) {
                goal = current;
                found = true;
                break;
            }

            auto add_state = [&](SearchState next, int next_g) {
                auto it = g_score.find(next);
                if (it == g_score.end() || next_g < it->second) {
                    g_score[next] = next_g;
                    parent[next] = current;
                    open.push({next_g + heuristic(next.position), next});
                }
            };

            // Switching the ring changes safety but does not add a movement step.
            if (!mind_map.get_coat_state()) {
                SearchState switched{current.position, !current.ring_on};
                bool safe = switched.ring_on
                    ? is_valid_move_with_ring(switched.position)
                    : is_valid_move_without_ring(switched.position);
                if (safe) add_state(switched, current_g);
            }

            vector<Position> moves;
            moves.emplace_back(current.position.x, current.position.y + 1);
            moves.emplace_back(current.position.x + 1, current.position.y);
            moves.emplace_back(current.position.x, current.position.y - 1);
            moves.emplace_back(current.position.x - 1, current.position.y);

            for (auto &mv : moves) {
                bool safe = current.ring_on
                    ? is_valid_move_with_ring(mv)
                    : is_valid_move_without_ring(mv);
                if (safe) add_state({mv, current.ring_on}, current_g + 1);
            }
        }

        if (!found) return false;

        // Recover the first physical move from the optimal path on the known map.
        vector<SearchState> path;
        for (SearchState state = goal; !(state == start); state = parent[state]) {
            path.push_back(state);
        }
        for (auto it = path.rbegin(); it != path.rend(); ++it) {
            if (it->position == frodo_pos) continue;
            if (it->ring_on != is_ring_on()) {
                toggle_ring();
                cout << (is_ring_on() ? "r" : "rr") << endl;
                cout.flush();
            }
            frodo_pos = it->position;
            total_moves++;
            cout << "m " << frodo_pos.x << " " << frodo_pos.y << endl;
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
        bool step_success = game.make_astar_step(gollum_pos);

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

    // Read Mount Doom position after finding Gollum
    int md_x, md_y;
    cin >> md_x >> md_y;

    Position mount_doom_pos = Position(md_x, md_y);

    game.reset_for_new_phase();

    bool path_to_mount_doom_exists = true;
    steps_since_progress = 0;

    // Phase 2: Reach Mount Doom
    while (game.get_current_pos() != mount_doom_pos) {
        Position old_pos = game.get_current_pos();
        bool step_success = game.make_astar_step(mount_doom_pos);

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
}
