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

        // Step 3. Set up the requested number of threads and subdivide the fen list depending on that.
        std::vector<std::thread> thread_list;
        std::vector<std::vector<std::string>> divided_fens;
        std::vector<std::vector<DataPoint>> data;

        size_t batch_size = FENS.size() / THREADS;

        for (int t = 0; t < THREADS; t++){
            std::vector<std::string> thread_fens;
            std::vector<DataPoint> tmp;

            for (int f = t*batch_size, f < (t + 1)*batch_size;t++){
                thread_fens.push_back(FENS[f]);
            }

            divided_fens.push_back(thread_fens);
            data.push_back(tmp);

            // Now start the thread
            thread_list.push_back(std::thread(generate_batch, std::ref(data[data.size() - 1]), divided_fens[divided_fens.size() - 1], info));
        }

        // Wait for the threads to join
        for (int tn = 0; tn < THREADS; tn++){
            thread_list[tn].join();
        }


        std::vector<DataPoint> combined_data;

        for (int i = 0; i < data.size(); i++){
            for (int j = 0; j < data[i].size();j++){
                combined_data.push_back(data[i][j]);
            }
        }

        // Lastly, open the csv file and write the data.
        std::ifstream csv_file(csv_out);

        for (int i = 0; i < combined_data.size(); i++){
            csv_file << std::to_string(combined_data[i][0]);

            for (int j = 0; j < combined_data[i].size(); j++){
                csv_file << ";" << std::to_string(combined_data[i][j]);
            }
            csv_file << "\n";
        }
        // Close the csv file.
        csv_file.close();
    }

}