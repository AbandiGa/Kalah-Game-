#include <iostream>
#include <chrono>
#include "PlayGame.h"
#include <cstdlib>
#include <fstream>

// ./a.out alg[1] heu[1] alg[0] heu[0] max_depth[1] max_depth[0] diagout.csv
//        alg[1]: algorithm for max player, 0 for rich/knight, 1 for norvig/luger
//        heu[1]: heuristic for max player, -1 for test, 0 for alabandi, 1 for bell, 2 for coplin
//        alg[0]: analogous to alg_max but for min player
//        heu[0]: analogous to heu_max but for min player
//  max_depth[1]: maximum depth used by max's search tree
//  max_depth[0]: maximum depth used by min's search tree
//   diagout.csv: filename for diagnostic output

void printboard(std::vector<int> field);
bool game_over(std::vector< int >);
void wait_for_user();
void output_user_info(bool player_max, bool alg, int heuristic, int depth);

int main(int argc, char* argv[]) {
    const char *h_name[4];
    h_name[0] = "Alabandi's H"; h_name[1] = "Bell's H";
    h_name[2] = "Coplin's H"; h_name[3] = "The simple h";
    std::vector< bool > alg;
    std::vector< int > heu;
    std::vector< int > max_depth;
    std::ofstream diag;
    bool is_player_one = 1;
    if(argc != 8){
        alg.push_back(1);
        alg.push_back(1);
        heu.push_back(3);
        heu.push_back(3);
        max_depth.push_back(6);
        max_depth.push_back(2);
        diag.open("test.csv");
    } else{
        alg.push_back(atoi(argv[3]));
        alg.push_back(atoi(argv[1]));
        heu.push_back(atoi(argv[4]));
        heu.push_back(atoi(argv[2]));
        max_depth.push_back(atoi(argv[6]));
        max_depth.push_back(atoi(argv[5]));
        diag.open(argv[7]);
    }
    if(diag.is_open()) {
        diag << "Move Index,Max's Score,Min's Score,Children Generated,Move Made,Time to Run,Board,Path,H Score" << std::endl;
    }

    std::cout << "Kalah game!" << std::endl;
    output_user_info(1, alg[1], heu[1], max_depth[1]);
    output_user_info(0, alg[0], heu[0], max_depth[0]);
    std::cout << std::endl << std::endl;

    std::vector< int > board;
    for(int i = 0; i < 14; i++){
        if(i%7 == 6) board.push_back(0);
        else board.push_back(6);
    }
    std::cout << "Player " << 2 - is_player_one << "'s turn" << std::endl;
    printboard(board);

    int move_count = 1;
    while(!game_over(board)){
        std::chrono::high_resolution_clock::time_point time_before = std::chrono::high_resolution_clock::now();
        PlayGame next_move(board, alg[is_player_one], is_player_one, heu[is_player_one], max_depth[is_player_one]);
        std::chrono::high_resolution_clock::time_point time_after = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> seconds_used = std::chrono::duration_cast<std::chrono::duration<double>>(time_after - time_before);
        std::cout << "Player " <<  2 - is_player_one << " generated " << next_move.children_generated << " children in " << seconds_used.count() << " seconds." << std::endl;
        std::cout << h_name[heu[is_player_one]] << "euristic Move Score: " << next_move.heuristic_score << std::endl;
        std::cout << "Predicted Path: ";
        next_move.output_path();
        std::cout << std::endl << std::endl;
        if(diag.is_open()) {
            diag << move_count << ",";
            diag << next_move.next_moves_board[6] << ",";
            diag << next_move.next_moves_board[13] << ",";
            diag << next_move.children_generated << ",";
            diag << next_move.move[0];
            for(int i = 1; i < next_move.move.size(); i++){
                diag << " " << next_move.move[i];
            }
            diag << "," <<seconds_used.count() << ",";
            diag << next_move.next_moves_board[0];
            for(int i = 1; i < 14; i++){
                diag << " " << next_move.next_moves_board[i];
            }
            diag << ",";
            next_move.output_path(diag);
            diag << "," << next_move.heuristic_score << std::endl;
        }
        is_player_one = !is_player_one;
        board.clear();
        for(int i = 0; i < 14; i++){
            board.push_back(next_move.next_moves_board[i]);
        }
        std::cout << std::endl;
        std::cout << "Player " << 2 - is_player_one << "'s turn" << std::endl;
        printboard(board);
        std::cout << std::endl;
        move_count++;
    }
    if(board[6] > board[13]) std::cout << "Player 1 wins!" << std::endl;
    else if(board[6] < board[13]) std::cout << "Player 2 wins!" << std::endl;
    else std::cout << "Draw!" << std::endl;
    if(diag.is_open()) diag.close();
    wait_for_user();
    return 0;
}

bool game_over(std::vector< int > board){
    //if someone scores half of the board, the game is over.
    if(board[6] > 36 || board[13] > 36) return true;

    for(int i = 0; i < 6; i++){
        if(board[i] != 0) return false;
        if(board[12 - i] != 0) return false;
    }
    return true;
}

void printboard(std::vector<int> field)
{
    int pits = 6;
    using namespace std;
    //Default board
    //MIN: 12|11|10|9 |8 |7 |
    // -------------------------
    // |  |6 |6 |6 |6 |6 |6 |  |
    // |0 |--+--+--+--+--+--|0 |
    // |  |6 |6 |6 |6 |6 |6 |  |
    // -------------------------
    //MAX: 0 |1 |2 |3 |4 |5 |

    //line 1
    cout << "MIN: ";
    for(int i=(2*pits); i>pits; i--) {
        if (i < 10) {
            cout <<i<<" |";
        } else {
            cout <<i<<"|";
        }
    }
    cout<<endl;
    //line 2
    cout << " ----";
    for(int i=pits; i>0; i--) {
        cout << "---";
    }
    cout << "---" <<  endl;
    //line 3
    cout << " |  |";
    for(int i=(2*pits); i>pits; i--) {
        cout << field[i];
        if (field[i]<10) {
            cout << " |";
        } else {
            cout << "|";
        }
    }
    cout << "  |" << endl;
    //line 4
    cout << " |" << field[(2*pits+1)];
    if (field[2*pits+1] < 10) {
        cout << " |";
    } else {
        cout << "|";
    }
    for(int i=pits; i>1; i--) {
        cout << "--+";
    }
    cout << "--|" << field[pits];
    if (field[pits]<10) {
        cout << " |";
    } else {
        cout << "|";
    }
    cout << endl;
    //line 5
    cout << " |  |";for(int i=0; i<pits; i++) {
        cout << field[i];
        if (field[i]<10) {
            cout << " |";
        } else{
            cout << "|";
        }
    }
    cout << "  |" << endl;
    //line 6
    cout << " ----";
    for(int i=pits; i>0; i--) {
        cout << "---";
    }
    cout << "---" << endl;
    //line 7
    cout << "MAX: ";
    for(int i=0; i<pits; i++) {
        if (i < 10) {
            cout <<i<<" |";
        } else {
            cout <<i<<"|";
        }
    } cout<<endl;
}

void wait_for_user(){
    std::cout << "Press enter to continue. . ." << std::endl;
    std::cin.ignore(std::numeric_limits< std::streamsize >::max(), '\n');
}

void output_user_info(bool player_max, bool alg, int heuristic, int depth){
    using namespace std;
    const char *h_name[4];
    h_name[0] = "Alabandi's"; h_name[1] = "Bell's";
    h_name[2] = "Coplin's"; h_name[3] = "the simple";
    cout << "Player " << 2 - player_max << " is using " << h_name[heuristic] << " heuristic in ";
    alg ? cout << "Norvig and Luger's " : cout << "Rich and Knight's ";
    cout << "minimax algorithm, with a cutoff depth of " << depth << "." << endl;
}