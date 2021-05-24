#include "generate.h"


namespace DataGeneration {

    void generate_batch(std::vector<DataPoint>* data, const std::vector<std::string>& FENS, GenerationInfo info, bool main_thread){

        // Step 1. Clear the data vector, declare a position object and an array for holding the network input.
        data->clear();
        GameState_t* pos = new GameState_t;
        int current_filerow = 0;

        // Step 2. Loop through all the positions
        for (int p = 0; p < FENS.size(); p++){
            DataPoint dp;
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

            data->push_back(dp);

            if (main_thread && p % 10000 == 0) {
                std::cout << "Generated " << p << "/" << FENS.size() << " positions" << std::endl;
            }
        }

        // Step 3. Delete pos and return
        delete pos;
    }


    // Main method.
    void generate_training_data(std::string epd_in, std::string csv_out, bool use_search, int depth){
        // Step 1. Open the epd file, parse all the FENS and set up a generation data object.
        std::vector<std::string> FENS;
        std::string fen, epd;
        std::ifstream epd_file(epd_in);

        GenerationInfo info;
        info.search_scores = use_search;
        info.search_depth = depth;

        // Step 2. Extract the FENS from the epd file.
        while (std::getline(epd_file, epd)){
            auto fen_end = epd.find_first_of('"');

            fen = epd.substr(0, fen_end);
            FENS.push_back(fen);
        }
        epd_file.close();

        std::cout << "Extracted " << FENS.size() << " fens" << std::endl;

        // Step 3. Set up the requested number of threads and subdivide the fen list depending on that.
        /*std::vector<std::thread> thread_list;
        std::vector<std::vector<std::string>> divided_fens;
        //std::vector<std::vector<DataPoint>> data;

        //size_t batch_size = FENS.size() / THREADS;


        std::vector<DataPoint> data[THREADS];
        std::vector<std::string> thread_fens[THREADS];
        size_t batch_size = FENS.size() / THREADS;
        int current = 0;

        for (int t = 0; t < THREADS; t++) {
            int i = 0;

            while (i <= batch_size && (i + current) < FENS.size()) {
                thread_fens[t].push_back(FENS[current + i]);
                i++;
            }
            current += i;
        }

        for (int t = 0; t < THREADS; t++) {
            thread_list.push_back(std::thread(generate_batch,
                std::ref(data[t]),
                std::ref(thread_fens[t]),
                info,
                (t == 0) ? true : false
            ));
        }

        for (int t = 0; t < THREADS; t++) {
            thread_list[t].join();
        }*/

        std::ofstream csv_file(csv_out);

        std::vector<DataPoint>* combined_data = new std::vector<DataPoint>;
        combined_data->reserve(FENS.size());
        generate_batch(combined_data, FENS, info, true);

        //for (int t = 0; t < THREADS; t++) {
        //    for (int i = 0; i < data[t].size(); i++) {
        //        combined_data.push_back(data[t][i]);
        //    }
        //}

        // Lastly, open the csv file and write the data.
        //std::ofstream csv_file(csv_out);

        for (int i = 0; i < combined_data->size(); i++){
            int8_t* net_inp = (*combined_data)[i].network_input;
        
            csv_file << std::to_string(net_inp[0]);
        
            for (int j = 1; j < INPUT_SIZE; j++){
                csv_file << ";" << std::to_string(net_inp[j]);
            }
        
            csv_file << ";" << std::to_string((*combined_data)[i].score);
        
            csv_file << "\n";

            if (i % 100000 == 0) {
                std::cout << "Wrote " << i << "/" << FENS.size() << " positions to the CSV file" << std::endl;
            }
        }
        // Close the csv file.
        csv_file.close();
    }

}