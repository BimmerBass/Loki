#include "generate.h"


namespace DataGeneration {

    void generate_batch(std::vector<Data::DataEntry>& data, const std::vector<std::string>& FENS, GenerationInfo info, bool main_thread){

        // Step 1. Clear the data vector, declare a position object and an array for holding the network input.
        data.clear();
        GameState_t* pos = new GameState_t;
        //SearchThread_t* ss = new SearchThread_t;
        //SearchInfo_t* s_info = new SearchInfo_t;
        int current_filerow = 0;

        // Step 2. Loop through all the positions
        for (int p = 0; p < FENS.size(); p++){
            Data::DataEntry dp;
            // Step 2A. Parse the FEN
            pos->parseFen(FENS[p]);
            int score = 0;

            // Step 2B. If we're told to use search, set up a searchthread.
            //if (info.search_scores){
            //    SearchThread_t ss;
            //    SearchInfo_t search_info;
            //    
            //    // Step 2B.1. Set up the search info and a pvLine since alphabeta requires this to be given.
            //    search_info.timeset = false;
            //    search_info.depth = info.search_depth;
            //    SearchPv pv;
            //
            //    // Step 2B.2. Set up the searchthread and run an alphabeta search
            //    *ss.pos = *pos;
            //    *ss.info = search_info;
            //
            //    score = Search::alphabeta(&ss, search_info.depth, -INF, INF, true, &pv);
            //}
            //else{
            //    score = Eval::evaluate(pos);
            //}
            //score = Eval::evaluate(pos);
            //pos->use_lnn = false;
            //*(ss->pos) = *pos;
            //*(ss->info) = *s_info;
            //
            //score = Search::quiescence(ss, -INF, INF);
            score = Eval::evaluate(pos);

            if (pos->side_to_move == BLACK) { score *= -1; }

            // Step 2C. Set up the array of the position.
            Bitboard pceBoard = 0;
            memset(dp.network_input, 0, sizeof(int8_t) * INPUT_SIZE);

            for (int pce = PAWN; pce <= KING; pce++){
                pceBoard = pos->pieceBBS[pce][WHITE];

                while (pceBoard){
                    int sq = PopBit(&pceBoard);

                    dp.network_input[LNN::calculate_input_index(pce, true, sq)] = 1;
                }
            }
            for (int pce = PAWN; pce <= KING; pce++){
                pceBoard = pos->pieceBBS[pce][BLACK];

                while (pceBoard){
                    int sq = PopBit(&pceBoard);

                    dp.network_input[LNN::calculate_input_index(pce, false, sq)] = 1;
                }
            }

            // Step 2D. Now we can set up a DataPoint and push it to the vector.
            dp.score = score;

            data.push_back(dp);

            if (main_thread && p % 10000 == 0) {
                std::cout << "Generated " << p << "/" << FENS.size() << " positions" << std::endl;
            }
        }

        // Step 3. Delete pos and return
        delete pos;
        //delete ss, s_info;
    }


    // Main method.
    void generate_training_data(std::string epd_in, std::string lgd_out, bool use_search, int depth){
        // Step 1. Open the epd file, parse all the FENS and set up a generation data object.
        std::vector<std::string> FENS;
        std::string fen, epd;
        std::ifstream epd_file(epd_in);

        GenerationInfo info;
        info.search_scores = use_search;
        info.search_depth = depth;

        // Step 2. Extract the FENS from the epd file.
        int fen_count = 0;
        while (std::getline(epd_file, epd)){
            auto fen_end = epd.find_first_of('"');

            fen = epd.substr(0, fen_end);
            FENS.push_back(fen);

            fen_count++;

            if (fen_count % 10000 == 0) {
                std::cout << "Extracted " << fen_count << " fens" << std::endl;
            }
        }
        epd_file.close();

        std::cout << "Extracted " << FENS.size() << " fens" << std::endl;


        // Step 3. Now set up a data writer object which will create our output file.
        Data::DataWriter writer(lgd_out);

        // Step 4. Determine whether the file should be written to in batches or at once.
        bool incremental_writing = true;

        if (FENS.size() <= MAX_BATCH_SIZE) {
            incremental_writing = false;
        }

        // Step 5. We can now generate the data.
        // If we're using incremental writing we also need to determine the batches.
        std::vector<Data::DataEntry> generated_data;

        if (incremental_writing) {

            // Step 5A. Partition the FENS.
            std::vector<size_t> batches;

            size_t batch_count = FENS.size() / MAX_BATCH_SIZE;
            size_t remainder = FENS.size() % MAX_BATCH_SIZE;

            for (int i = 0; i < batch_count; i++) {
                batches.push_back(i * MAX_BATCH_SIZE);
            }
            if (remainder > 0) {
                batches.push_back(batches.back() + MAX_BATCH_SIZE);
            }
            assert(batches.back() < FENS.size());

            batches.push_back(FENS.size());

            std::vector<std::string> fen_batch;
            // 5B. Now we can generate and load the divided data.
            for (int b = 0; b < batches.size() - 1; b++) {

                // Step 5B.1. Copy all the FENS we're going to use.
                fen_batch.clear(); fen_batch.reserve(MAX_BATCH_SIZE);
                for (int i = batches[b]; i < batches[b + 1]; i++) {
                    fen_batch.push_back(FENS[i]);
                }

                // Step 5B.2. Generate data for the batch.
                generated_data.clear(); generated_data.reserve(MAX_BATCH_SIZE);
                generate_batch(generated_data, fen_batch, info, true);

                // Step 5B.3. Now write this data to the file.
                writer.save_data(generated_data);
            }
        }
        else {
            // Step 5A. Generate all data at once.
            generated_data.reserve(FENS.size());
            generate_batch(generated_data, FENS, info, true);

            // Step 5B. Now make the writer object write this to a file.
            writer.save_data(generated_data);
        }
    }

}