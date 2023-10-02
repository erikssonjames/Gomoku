#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <cassert>
#include <queue>

#include "gomoku.h"
#include "Timer.h"

std::string get_game_state(const GameState &gameState)
{
    switch (gameState) {
        case GameState::white_wins:
            return "White wins";
        case GameState::black_wins:
            return "Black wins";
        case GameState::active:
            return "Active";
        case GameState::draw:
            return "Draw";
    }

    return "";
}

std::pair<std::string, std::string> get_player_data(std::string& s)
{
    std::string::size_type start = s.find_first_of(';');
    std::string::size_type end = s.find_last_of(';');

    std::string username = s.substr(start + 1, end - (start + 1));
    std::string realname = s.substr(end + 1, s.length() - end);

    return std::make_pair(username, realname);
}

void init_players(const char *file_name, GomokuBackend& backend)
{
    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }

    std::string word;
    while (file >> word) {
        auto [username, realname] = get_player_data(word);
        backend.add_player(username, realname);
    }

    file.close();
}

std::vector<std::queue<Square>> init_games(const char* file_name, GomokuBackend& backend)
{
    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return {};
    }

    std::vector<std::queue<Square>> read_games{};
    read_games.reserve(1000);

    std::string word;
    while (file >> word) {
        std::stringstream ss(word);
        std::string item;

        GameID game_id{};
        PlayerID black{};
        PlayerID white{};
        int number_of_moves{};
        GameState gameState;
        uint8_t index = 0;
        while (index < 5 && std::getline(ss, item, ',')) {
            const int num = std::stoi(item);

            switch (index) {
                case 0:
                    game_id = num;
                    break;
                case 1:
                    black = num;
                    break;
                case 2:
                    white = num;
                    break;
                case 3:
                    number_of_moves = num;
                    break;
                case 4:
                    if (num == 0) {
                        gameState = GameState::active;
                    } else if (num == 1) {
                        gameState = GameState::black_wins;
                    } else if (num == 2) {
                        gameState = GameState::white_wins;
                    } else {
                        gameState = GameState::draw;
                    }
                    break;
            }

            index++;
        }

        assert(game_id != 0);

        read_games.emplace_back();

        std::string xS{}, yS{};
        while (std::getline(ss, yS, ',') && std::getline(ss, xS, ',')) {
            const uint8_t x = std::stoul(xS);
            const uint8_t y = std::stoul(yS);

            Square sq{.x =  x, .y =  y};
            read_games[game_id - 1].push(sq);
        }

        GameID created_game_id = backend.start_game(black, white);
        assert(game_id == created_game_id);
    }

    file.close();

    return read_games;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    uint32_t total_number_of_moves{};
    double total_seconds_taken{};
    const int NUM_ITERATIONS = 100;
    for (int iteration = 0; iteration < NUM_ITERATIONS; iteration++) {
        GomokuBackend backend;

        init_players("C:\\Users\\jamer\\CLionProjects\\Gomoku\\players.txt", backend);
        auto games = init_games("C:\\Users\\jamer\\CLionProjects\\Gomoku\\games.txt", backend);

        Timer tmr;
        tmr.start();

        size_t active_games = games.size();
        int index = 0;
        int moves = 0;
        while (active_games > 0) {
            GameID game_id = index + 1;
            auto &queue = games[index];

            if (++index >= games.size()) index = 0;

            if (!queue.empty()) {
                Square move = queue.front();
                queue.pop();
                auto game_state = backend.perform_move(game_id, move);
                moves++;

                if (queue.empty() || game_state != GameState::active) {
                    active_games--;
                    queue = {};
                }
            }
        }

        tmr.stop();

        total_seconds_taken += tmr.seconds();
        total_number_of_moves += moves;
    }

    std::cout << "Running " << NUM_ITERATIONS << " iterations took a total of " << total_seconds_taken << "s" << "\n";
    std::cout << "A total of " << total_number_of_moves << " moves were made" << "\n";
    std::cout << "This equates to a throughput of " << total_number_of_moves / total_seconds_taken << " moves/s" << "\n";

    return 0;
}
