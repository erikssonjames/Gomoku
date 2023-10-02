#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

using GameID = uint32_t;
using PlayerID = uint32_t;

enum class GameState { active, black_wins, white_wins, draw };
enum class SquareState { empty, black, white };

struct Player {
    std::string username;
    std::string realname;
};

struct Square { 
    uint8_t x;
    uint8_t y; 
};

struct Position {
    SquareState grid[15][15]{ SquareState::empty };

    void        set(int y, int x, SquareState c) { grid[y][x] = c; }
    SquareState get(int y, int x) const          { return grid[y][x]; }
};

struct Game {
    PlayerID black;
    PlayerID white;
    std::vector<Square> moves;
    Position position;
    GameState state;
};

class GomokuBackend
{
public:
    // play games
    GameID              start_game(PlayerID black, PlayerID white);
    std::vector<Square> get_legal_moves(GameID id) const;
    GameState           perform_move(GameID id, Square sq);
    Position            get_position(GameID id) const;

    // manage players
    std::vector<PlayerID> get_player_ids() const;
    PlayerID              add_player(std::string username, std::string realname);
    PlayerID              get_player_id(std::string username) const;
    Player                get_player(PlayerID id) const;
    Player                get_player(std::string username) const;
    
    // manage games
    std::vector<GameID> get_game_ids() const;
    Game                get_game(GameID id) const;
    std::vector<GameID> get_games_played_by(PlayerID player) const;
    std::vector<GameID> find_games_with_position(const Position& pos) const;

    GameState           check_game_state(const Game &game, int y0, int x0);
    void                print_game(Game &game);
private:
    SquareState         get_next_square_state(const Game &game);
    bool                check_row(const Position& p, int y0, int x0);
    std::string         position_to_string(const Position& p) const;
private:
    uint32_t game_counter = 1;
    std::unordered_map<GameID, Game> games_{};
    std::unordered_map<PlayerID, Player> players_{};
    std::unordered_map<PlayerID, std::vector<GameID>> player_games_{};
    std::unordered_map<std::string , std::vector<GameID>> game_states_{};
};
