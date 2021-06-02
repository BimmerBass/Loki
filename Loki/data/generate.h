#ifndef GENERATE_H
#define GENERATE_H

#include "../search.h"
#include "../lnn/network.h"
#include "../lnn/trainer/data.h"


namespace DataGeneration {
    // The number of threads to use.
    constexpr int THREADS = 8;
    static_assert(THREADS >= 1);

    // The maximal batch size to use for writing incrementally.
    constexpr size_t MAX_BATCH_SIZE = 5000000;
    static_assert(MAX_BATCH_SIZE > 0);

    
    // This struct holds all data that represents one position.
    struct SavedBoard {
        // Board state
        Bitboard pieces[6][2] = { {0} };
        SIDE stm = WHITE;
        int en_passant = NO_SQ;
        int castling_rights = 0;
        int move50 = 0;

        // Score
        int score = 0;

        void setup(const GameState_t* pos, int evaluation);
    };



    // This class is responsible for playing a predetermined number of games on one thread.
    // Each thread will get their own
    class Arbiter {
    public:
        Arbiter(size_t _count, SearchInfo_t* _settings, bool play_random = false);
        ~Arbiter();

        // Runs the specified number of games.
        void run();

    private:
        // Used for random position generation.
        const bool random_play;

        GameState_t* board;
        SearchThread_t* searcher;

        // Play one game.
        void play_game();

        // Holds all positions that we have reached.
        // Note: Ealy opening (<= move 3) and checkmate/stalemate positions are left out.
        std::vector<SavedBoard> positions;
    };
}



#endif