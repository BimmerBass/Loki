#include "generate.h"


namespace DataGeneration {

    void generate_batch(std::vector<DataPoint>& data, std::vector<std::string> FENS, GenerationInfo info){

        // Step 1. Clear the data vector, declare a position object and an array for holding the network input.
        data.clear();
        GameState_t* pos = new GameState_t;
        std::array<neuron_t, INPUT_SIZE> net_input;


        // Step 2. Loop through all the positions
        for (int p = 0; p < FENS.size(); p++){
            // Step 2A. Parse the FEN
            pos->parseFen(FENS[p]);
            int score = 0;

            // Step 2B. If we're told to use search, set up a searchthread.
            if (info.search_scores){
                SearchThread_t ss;
                SearchInfo_t search_info;
                
                // Step 2B.1. Set up the search info and a pvLine since alphabeta requires this to be given.
                search_info.timeset = false;
                search_info.depth = info.search_depth;
                SearchPv pv;

                // Step 2B.2. Set up the searchthread and run an alphabeta search
                *ss.pos = *pos;
                *ss.info = search_info;

                score = Search::alphabeta(ss, search_info.depth, -INF, INF, true, &pv);
            }
            else{
                score = Eval::evaluate(pos);
            }

            // Step 2C. Set up the array of the position.
            Bitboard pceBoard = 0;
            net_input.fill(neuron_t(0));
            for (int pce = PAWN; pce <= KING; pce++){
                pceBoard = pos->pieceBBS[pce][WHITE];

                while (pceBoard){
                    int sq = PopBit(&pceBoard);

                    net_input[LNN::calculate_input_index(pce, true, sq)] = neuron_t(1);
                }
            }
            for (int pce = PAWN; pce <= KING; pce++){
                pceBoard = pos->pieceBBS[pce][BLACK];

                while (pceBoard){
                    int sq = PopBit(&pceBoard);

                    net_input[LNN::calculate_input_index(pce, false, sq)] = neuron_t(1);
                }
            }

            // Step 2D. Now we can set up a DataPoint and push it to the vector.
            DataPoint dp;
            dp.network_input = net_input;
            dp.score = static_cast<neuron_t>(score);

            data.push_back(dp);
        }

        // Step 3. Delete pos and return
        delete pos;
    }


}