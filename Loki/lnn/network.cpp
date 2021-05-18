#include "network.h"


/*

Compute the dot-product between two vectors (std::array).

*/
template<typename T, size_t SIZE>
void dot_product(const std::array<T, SIZE>& v1, const std::array<T, SIZE>& v2, T& out) {

	out = 0;

	// A vector dot product is just a sum of element-wise multiplications
	for (int i = 0; i < SIZE; i++) {
		out += v1[i] * v2[i];
	}
}

// ReLU activation function
template<typename T>
T apply_ReLU(T v) {
	return std::max(T(0), v);
}

namespace LNN {

	/*
	
	Load a position into the input. This will be done when we're given the position by the GUI.
	Afterwards we'll use the incremental update methods for speed.

	*/
	void Network::load_position(std::array<int8_t, INPUT_SIZE>& pos) {

		// Loop through the inputs
		for (int i = 0; i < INPUT_SIZE; i++) {
			// Make sure we only have allowed values in the input array
			assert(pos[i] == 0 || pos[i] == 1);

			(INPUT_LAYER.back()).neurons[i] = static_cast<neuron_t>(pos[i]);
		}
	}


	/*
	
	Evaluate a position through forward-propagation. This is the main method used in the network class.
	The parameter "fast" is used to determine how much of the net should be calculated.
		- true: Calculate from the first hidden layer, which is already calculated itself from the inverse updates.
		- false: Calculate the entire network from the input
	*/
	int Network::evaluate(bool fast) {

		// Step 1. If we're told to do a full calculation, compute the first hidden layer
		if (!fast) {

			for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {

				// Step 1A. Calculate the dot product of the input neurons and the weights between the layers
				dot_product<neuron_t, INPUT_SIZE>((INPUT_LAYER.back()).neurons, (INPUT_LAYER.back()).weights[i], FIRST_HIDDEN.neurons[i]);

				// Add bias
				FIRST_HIDDEN.neurons[i] += FIRST_HIDDEN.biases[i];
			}
		}

		// Step 1B. For each neuron in the hidden layer, apply the activation function (do_incremental doesn't do this, so we need to do this extra loop here)
		for (int i = 0; i < FIRST_HIDDEN_SIZE; i++) {
			FIRST_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(FIRST_HIDDEN.neurons[i]);
		}

		// Step 2. Now calculate all other layers starting with first hidden to second hidden
		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			dot_product<neuron_t, FIRST_HIDDEN_SIZE>(FIRST_HIDDEN.neurons, FIRST_HIDDEN.weights[i], SECOND_HIDDEN.neurons[i]);

			SECOND_HIDDEN.neurons[i] += SECOND_HIDDEN.biases[i];
			SECOND_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(SECOND_HIDDEN.neurons[i]);
		}

		for (int i = 0; i < HIDDEN_STD_SIZE; i++) {
			dot_product<neuron_t, HIDDEN_STD_SIZE>(SECOND_HIDDEN.neurons, SECOND_HIDDEN.weights[i], THIRD_HIDDEN.neurons[i]);

			THIRD_HIDDEN.neurons[i] += THIRD_HIDDEN.biases[i];
			THIRD_HIDDEN.neurons[i] = apply_ReLU<neuron_t>(THIRD_HIDDEN.neurons[i]);
		}
		

		// Step 3. Now calculate the output, but don't apply an activation function or a bias
		dot_product<neuron_t, HIDDEN_STD_SIZE>(THIRD_HIDDEN.neurons, THIRD_HIDDEN.weights[0], OUTPUT_LAYER.neurons[0]);

		return static_cast<int>(OUTPUT_LAYER.neurons[0]);
	}


	/*
	Make a move incrementally
	*/
	void Network::do_incremental() {
		// Step 1. Update the input layer
		// Copy the current input layer and add it to the back of the vector
		INPUT_LAYER.push_back(INPUT_LAYER.back());

		for (int i = 0; i < network_updates.changes; i++) {
			// Make sure we dont update something impossible
			assert(((INPUT_LAYER.back()).neurons[network_updates.updates[i].index] == 1 && network_updates.updates[i].delta == -1)
				|| ((INPUT_LAYER.back()).neurons[network_updates.updates[i].index] == 0 && network_updates.updates[i].delta == 1));
			// Make the update
			(INPUT_LAYER.back()).neurons[network_updates.updates[i].index] += network_updates.updates[i].delta;
		}

		// Step 2. Propagate these updates to the hidden layer
		for (int i = 0; i < network_updates.changes; i++) {

			for (int j = 0; j < FIRST_HIDDEN_SIZE; j++) {
				if (network_updates.updates[i].delta == 1) { // Add
					FIRST_HIDDEN.neurons[j] += (INPUT_LAYER.back()).weights[j][network_updates.updates[i].index];
				}
				else { // Subtract.
					FIRST_HIDDEN.neurons[j] -= (INPUT_LAYER.back()).weights[j][network_updates.updates[i].index];
				}
			}
		}

	}


	/*
	Undo a move. We just delete the latest change of the input layer
	*/
	void Network::undo_incremental() {
		INPUT_LAYER.pop_back();
	}


	// The following four functions are for parsing a move when finding the incremental update parameters
	int Update::fromSq(const unsigned int move) const {
		return (((move) >> (4)) & (63));
	}
	int Update::toSq(const unsigned int move) const {
		return ((move) >> (10));
	}
	int Update::special_flag(const unsigned int move) const {
		return ((move) & (3));
	}
	int Update::promotion_piece(const unsigned int move) const {
		return (((move) >> (2)) & (3));
	}


	/*
	
	This method sets up all changes that should be applied to the input layer

	*/
	void Update::calculate_update(const unsigned int move, const int piece_moved, const bool is_capture, const int piece_captured, const bool white_to_move) {

		changes = 0; // Start by assuming we don't want to change anything

		// Step 1. Prepare the piece moved to be removed from the origin square
		updates[changes].delta = -1;
		updates[changes].index = calculate_input_index(piece_moved, white_to_move, fromSq(move));
		changes++;


		// Step 2. Prepare a piece to be placed on the destination square.
		if (special_flag(move) != 0) { // Not a promotion. Add the piece moved to the destination
			updates[changes].delta = 1;
			updates[changes].index = calculate_input_index(piece_moved, white_to_move, toSq(move));
		}
		else { // Add the promotion piece.
			updates[changes].delta = 1;
			updates[changes].index = calculate_input_index(promotion_piece(move) + 1, white_to_move, toSq(move));
		}
		changes++;

		// Step 3. If the move is a castling move, the rook's move should be set in the last two entries
		if (special_flag(move) == 2) {

			int origin_sq;
			int dest_sq;
			bool king_side = toSq(move) > fromSq(move);
			if (white_to_move) {
				origin_sq = king_side ? 7 : 0;
				dest_sq = king_side ? 5 : 3;
			}
			else {
				origin_sq = king_side ? 63 : 56;
				dest_sq = king_side ? 61 : 59;
			}

			updates[changes].delta = -1;
			updates[changes].index = calculate_input_index(3, white_to_move, origin_sq);
			changes++;
			updates[changes].delta = 1;
			updates[changes].index = calculate_input_index(3, white_to_move, dest_sq);
		}
		else {
			// Step 4. If the move is a capture, set the captured piece to be removed
			if (is_capture) {
				updates[changes].delta = -1;
				updates[changes].index = calculate_input_index(piece_captured, !white_to_move, toSq(move));
				changes++;
			}
			else if (special_flag(move) == 1) { // En passant
				int capture_sq = (white_to_move) ? toSq(move) - 8 : toSq(move) + 8;

				updates[changes].delta = 1;
				updates[changes].index = calculate_input_index(0, !white_to_move, capture_sq);
			}
		}

		assert(changes <= 3);
	}
}