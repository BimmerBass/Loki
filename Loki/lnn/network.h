#ifndef NETWORK_H
#define NETWORK_H
#include <array>
#include <vector>
#include <cstdint>
#include <assert.h>
#include <string>
#include <iostream>

#include "types/layer.h"
#include "types/architecture.h"

namespace LNN {

    // Class for updating the network inputs incrementally. This is inspired from Halogen
    class Update {
    public:
        void calculate_update(const unsigned int move, const int piece_moved, const bool is_capture, const int piece_captured, const bool white_to_move);

        struct Change {
            int index = 0;  /* What index should be updated */
            int delta = 0;  /* What should be done to the value at the index */
        };

        // There are four ways a move can change the board:
        //  1. The origin square is emptied
        //  2. If the move is not a promotion, the piece moved will end on the destination square. Otherwise another piecetype will end there
        //  3. If a piece is captured, that piece will be removed from the board.
        //  4. If the move is a castling move, two pieces are moved simultaneously. The rook and king.
        Change updates[4];
        size_t changes = 0;
    private:
        int fromSq(const unsigned int move) const;
        int toSq(const unsigned int move) const;
        int special_flag(const unsigned int move) const;
        int promotion_piece(const unsigned int move) const;
    };


    /*
    Network class
    */
    class Network {
    public:
        Network();

        // The function used when evaluating the position
        int evaluate(bool fast = true);

        // The function for updating the inputs incrementally.
        void do_incremental();
        void undo_incremental();

        // Method for loading a position which will then be updated incrementally afterwards
        void load_position(std::array<int8_t, INPUT_SIZE>& pos);

        // Method for loading a network from a file. This should always be used at startup.
        //void load_net(std::string file_path);

        std::array<neuron_t, INPUT_SIZE> get_input() {
            return (INPUT_LAYER.back()).neurons;
        }

        Update network_updates;
    private:
        // For the network, we use the following architecture:
        //  768 input neurons --> There are 12 pieces with 64 squares each
        //  256 neurons in the first hidden layer
        //  32 neurons in the next hidden layer
        //  32 neurons in the last hidden layer
        //  1 output neuron since this is a value-based neural network
        std::vector<Layer<INPUT_SIZE, FIRST_HIDDEN_SIZE, neuron_t>> INPUT_LAYER;
        Layer<FIRST_HIDDEN_SIZE, HIDDEN_STD_SIZE, neuron_t> FIRST_HIDDEN;
        Layer<HIDDEN_STD_SIZE, HIDDEN_STD_SIZE, neuron_t> SECOND_HIDDEN;
        Layer<HIDDEN_STD_SIZE, OUTPUT_SIZE, neuron_t> THIRD_HIDDEN;
        Layer<OUTPUT_SIZE, 0, neuron_t> OUTPUT_LAYER;
    };

    constexpr int piece_conversion[2][6] = {
        {6, 7, 8, 9, 10, 11},
        {0, 1, 2, 3, 4, 5}
    };

    // piece_type should go from 0 to 5, and square should go from 0 to 63.
    inline int calculate_input_index(int piece_type, bool white, int sq) {
        return 64 * piece_conversion[(white) ? 1 : 0][piece_type] + sq;
    }
}


#endif