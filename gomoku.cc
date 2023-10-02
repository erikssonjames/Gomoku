#include <iostream>
#include <utility>
#include <bitset>
#include "gomoku.h"

GameID GomokuBackend::start_game(PlayerID black, PlayerID white)
{
    GameID id = game_counter++;
    Game game{black, white, {}, {}, GameState::active};

    games_.emplace(id, game);

    player_games_[black].push_back(id);
    player_games_[white].push_back(id);

    return id;
}

std::vector<Square> GomokuBackend::get_legal_moves(GameID id) const
{
    auto game = get_game(id);

    std::vector<Square> vec{};
    vec.reserve(15 * 15 - game.moves.size());

    for (uint8_t y = 0; y < 15; y++) {
        for (uint8_t x = 0; x < 15; x++) {
            if (game.position.get(y, x) == SquareState::empty) {
                vec.push_back({
                    x,
                    y
                });
            }
        }
    }

    return vec;
}

GameState GomokuBackend::perform_move(GameID id, Square sq)
{
    auto &game = games_[id];
    if (game.state != GameState::active) return game.state;

    game.moves.push_back(sq);
    game.position.set(sq.y, sq.x, get_next_square_state(game));
    game.state = check_game_state(game, sq.y, sq.x);

    auto position_string = position_to_string(game.position);

    if (auto search = game_states_.find(position_string);
        search != game_states_.end()) {
        search->second.push_back(id);
    } else {
        std::vector<GameID> vec{};
        vec.push_back(id);
        game_states_[position_string] = std::move(vec);
    }

    return game.state;
}

Position GomokuBackend::get_position(GameID id) const
{
    return get_game(id).position;
}

std::vector<PlayerID> GomokuBackend::get_player_ids() const
{
    std::vector<PlayerID> vec{};
    vec.reserve(players_.size());

    for (const auto& [key, _] : players_) {
        vec.push_back(key);
    }

    return vec;
}

PlayerID GomokuBackend::add_player(std::string username, std::string realname)
{
    PlayerID playerId = players_.size();
    Player player {std::move(username), std::move(realname)};
    players_.emplace(playerId, player);
    player_games_[playerId] = std::vector<GameID>{};
    return playerId;
}

PlayerID GomokuBackend::get_player_id(std::string username) const
{
    for (const auto& [key, value] : players_) {
        if (value.username == username) {
            return key;
        }
    }

    return {};
}

Player GomokuBackend::get_player(PlayerID id) const
{
    return players_.at(id);
}

Player GomokuBackend::get_player(std::string username) const
{
    return get_player(get_player_id(std::move(username)));
}

std::vector<GameID> GomokuBackend::get_game_ids() const
{
    std::vector<GameID> game_ids{};
    game_ids.reserve(game_ids.size());

    for (const auto& [game_id, _] : games_) {
        game_ids.push_back(game_id);
    }

    return game_ids;
}

Game GomokuBackend::get_game(GameID id) const
{
    return games_.at(id);
}

std::vector<GameID> GomokuBackend::get_games_played_by(PlayerID player) const
{

    if (auto search = player_games_.find(player);
            search != player_games_.end()) {
        return search->second;
    } else {
        std::cerr << "This player has not been added to the backend." << "\n";
        return {};
    }
}

std::vector<GameID> GomokuBackend::find_games_with_position(const Position &pos) const
{
    auto position_string = position_to_string(pos);

    if (auto search = game_states_.find(position_string);
        search != game_states_.end()) {
        return search->second;
    } else {
        return {};
    }
}




// Private

SquareState GomokuBackend::get_next_square_state(const Game &game)
{
    return game.moves.size() % 2 ? SquareState::black : SquareState::white;
}

bool GomokuBackend::check_row(const Position &p, int y0, int x0)
{
    SquareState sq = p.get(y0, x0);

    int horizontal_count{};
    const int xStart = x0 - 4 >= 0 ? x0 - 4 : 0;
    const int xEnd = x0 + 4 <= 14 ? x0 + 4 : 14;

    for (int x = xStart; x <= xEnd; x++) {
        if (p.get(y0, x) == sq) horizontal_count++;
        else horizontal_count = 0;

        if (horizontal_count == 5) return true;
    }

    int vertical_count{};
    const int yStart = y0 - 4 >= 0 ? y0 - 4 : 0;
    const int yEnd = y0 + 4 <= 14 ? y0 + 4 : 14;
    for (int y = yStart; y <= yEnd; y++) {
        if (p.get(y, x0) == sq) vertical_count++;
        else vertical_count = 0;

        if (vertical_count == 5) return true;
    }

    int count_lt_to_rb_diag = 0;
    int xDiag = x0 - 5;
    int yDiag = y0 - 5;
    for (int n = 0; n < 9; n++) {
        xDiag++;
        yDiag++;

        if (xDiag < 0 || yDiag < 0) continue;
        if (xDiag >= 15 || yDiag >= 15) break;

        if (p.get(yDiag, xDiag) == sq) count_lt_to_rb_diag++;
        else count_lt_to_rb_diag = 0;

        if (count_lt_to_rb_diag == 5) return true;
    }

    int count_lb_to_rt_diag = 0;
    xDiag = x0 - 5;
    yDiag = y0 + 5;
    for (int n = 0; n < 9; n++) {
        xDiag++;
        yDiag--;

        if (xDiag < 0 || yDiag >= 15) continue;
        if (xDiag >= 15 || yDiag < 0) break;

        if (p.get(yDiag, xDiag) == sq) count_lb_to_rt_diag++;
        else count_lb_to_rt_diag = 0;

        if (count_lb_to_rt_diag == 5) return true;
    }

    return false;
}

GameState GomokuBackend::check_game_state(const Game &game, int y0, int x0)
{
    if (game.moves.size() < 9) return GameState::active;

    if (check_row(game.position, y0, x0)) {
        return game.position.get(y0, x0) == SquareState::black ? GameState::black_wins : GameState::white_wins;
    }

    return game.moves.size() == 15 * 15 ? GameState::draw : GameState::active;
}

void GomokuBackend::print_game(Game &game) {
    std::cout << "------- PRINTING GAME ------" << "\n\n";

    for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 15; x++) {
            SquareState sq = game.position.get(y, x);

            switch (sq) {
                case SquareState::white:
                    std::cout << "W ";
                    break;
                case SquareState::black:
                    std::cout << "B ";
                    break;
                case SquareState::empty:
                    std::cout << "_ ";
            }
        }
        std::cout << "\n";
    }
}

std::string GomokuBackend::position_to_string(const Position &p) const
{
    std::string position_string{};
    position_string.reserve(15 * 15);

    for (int y = 0; y < 15; y++) {
        for (int x = 0; x < 15; x++) {
            SquareState sq = p.get(y, x);

            if (sq == SquareState::empty) {
                position_string += '_';
            } else if (sq == SquareState::black) {
                position_string += 'B';
            } else {
                position_string += 'W';
            }
        }
    }

    return position_string;
}
