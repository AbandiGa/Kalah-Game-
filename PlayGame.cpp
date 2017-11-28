//
// Created by Chris on 4/21/2017.
//

#include <iostream>
#include <fstream>
#include "PlayGame.h"

PlayGame::PlayGame(std::vector< int >& board, bool algorithm, bool player, int heuristic, int max_depth_parameter){
    //    Board: vector of size 14 representing the current board state
    //algorithm: 0 for Rich + Knight, 1 for Norvig and Luger
    //   player: 0 for min's turn, 1 for max's turn
    //heuristic: 0 for alabandi, 1 for bell, 2 for coplin, 3 for score difference
    std::unique_ptr< Node > dummy(new Node);
    root = std::move(dummy);
    root->board = board;
    root->depth = 0;
    root->player_max = player;
    root->parent = nullptr;
    function_used = heuristic;
    children_generated = 0;
    max_depth = max_depth_parameter;

    //run the game
    if(algorithm == 1) {
        move = alpha_beta_search(*root.get());

        next_moves_board = root->children[root->selected]->board;
        heuristic_score = root->children_value[root->selected];
        Node* cursor = root->children[root->selected].get();
        path.push_back(move);
        while(cursor->action.size() != 0){
            path.push_back(cursor->action[cursor->selected]);
            cursor = cursor->children[cursor->selected].get();
        }
    } else{
        minimax_a_b(*root.get(), 9999999999, -9999999999);
        move = root->action[root->selected];

        next_moves_board = root->children[root->selected]->board;
        heuristic_score = root->heuristic_value;
        Node* cursor = root->children[root->selected].get();
        path.push_back(move);
        while(cursor->selected != -1){
            path.push_back(cursor->action[cursor->selected]);
            cursor = cursor->children[cursor->selected].get();
        }

    }
} //the game is run during the constructor.

/******************************************************************************
 *  Alpha-Beta-Search from Russell and Norvig
 *****************************************************************************/
std::vector< int > PlayGame::alpha_beta_search(Node& state){
    double value;
    if(state.player_max){
        value = max_value(*root.get(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    } else{
        value = min_value(*root.get(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
    }
    for(int i = 0; i < root->action.size(); i++){
        if(root->children_value[i] == value){
            return root->action[i];
        }
    }
}

double PlayGame::max_value(Node& state, double alpha, double beta){
    if(cutoff_test(state)) return calculate_heuristic(state, function_used);
    double value = std::numeric_limits<double>::lowest();
    actions(state);
    for(int i = 0; i < state.action.size(); i++){
        generate_child(state, i);
        double temp_value = min_value(*(state.children[i].get()), alpha, beta);
        state.children_value.push_back(temp_value);
        if(value < temp_value){
            value = temp_value;
            state.selected = i;
        }
        if(value >= beta) {
            state.selected = i;
            return value;
        }
        alpha = (alpha > value) ? alpha : value;
    }
    return value;
}

double PlayGame::min_value(Node &state, double alpha, double beta) {
    if (cutoff_test(state)) return calculate_heuristic(state, function_used);
    double value = std::numeric_limits<double>::max();
    actions(state);
    for (int i = 0; i < state.action.size(); i++) {
        generate_child(state, i);
        double temp_value = max_value(*(state.children[i].get()), alpha, beta);
        state.children_value.push_back(temp_value);
        if (value > temp_value){
            value = temp_value;
            state.selected = i;
        }
        if (value <= alpha){
            state.selected = i;
            return value;
        }
        beta = (beta < value) ? beta : value;
    }
    return value;
}

bool PlayGame::cutoff_test(Node& state){
    //checks if the state has reached max_depth
    //or if state's board is an ended game
    if(state.depth == max_depth) return true;
    bool player_one_empty = true;
    bool player_two_empty = true;
    for(int i = 0; i < 6; i++){
        if(state.board[i] != 0) player_one_empty = false;
        if(state.board[i + 7] != 0) player_two_empty = false;
    }
    return player_one_empty || player_two_empty;
}

/******************************************************************************
 *  minimax_a_b; rich, knight
 *****************************************************************************/

//in text book, 3 values are passed to minimax and 2 are returned
// position is std::vector< int > node.board
// depth is int node.depth
// player is bool node.player_max
//the returned values are
// value is node.heuristic_value
// path is created via following a path in the constructor

void PlayGame::minimax_a_b(Node& node, double use_thresh, double pass_thresh){
    if(node.depth == max_depth || terminal_board(node.board)){
        node.heuristic_value = calculate_heuristic(node, function_used);
        //to correct for heuristic style
        if(!node.player_max) node.heuristic_value *= -1;
        node.selected = -1;
        return;
    }
    //generate successors
    actions(node);
    generate_children(node);
    for(int i = 0; i < node.children.size(); i++){
        Node* result_succ = node.children[i].get();
        minimax_a_b(*result_succ, -1 * pass_thresh, -1 * use_thresh);
        double new_value = -1*result_succ->heuristic_value;
        if(new_value > pass_thresh){
            pass_thresh = new_value;
            node.selected = i;
        }
        if(pass_thresh >= use_thresh){
            node.heuristic_value = pass_thresh;
            node.selected = i;
            return;
        }
    }
    node.heuristic_value = pass_thresh;
    return;
}

bool terminal_board(std::vector< int > board){
    for(int i = 0; i < 6; i++){
        if(board[i] != 0) return false;
        if(board[12 - i] != 0) return false;
    }
    return true;
}

/******************************************************************************
 *  Tree Functions
 *****************************************************************************/

void PlayGame::generate_children(Node& state){
    for(int i = 0; i < state.action.size(); i++){
        result(state, state.action[i]);
    }
}

void PlayGame::generate_child(Node& state, int i){
    result(state, state.action[i]);
}

void PlayGame::actions(Node& state){
    /*We need to find all possible moves that a player can make. This
      is more than 6 as it is possible to move more than once in a single
      turn. Each one of these can be represented by a chain of moves which
      is represented as a vector of integers corresponding to the jar
      indices that are used in the turn.*/

    //look at board from player's perspective
    std::vector< int > half_board;
    for(int i = 0; i < 6; i++){
        half_board.push_back(state.board[i + 7*!state.player_max]);
    }
    for(int i = 0; i < 6; i++){
        if(half_board[i] != 0){
            std::vector< int > current_action;
            current_action.push_back(i);
            if(half_board[i] + i == 6){
                actions_move_again(state, half_board, current_action);
                continue;
            }
            state.action.push_back(current_action);
        }
    }
    //because we only looked at the board on a single side, all moves will
    //be recorded as an integer from 0 to 5. This takes care of that.
    if(!state.player_max) {
        for (int i = 0; i < state.action.size(); i++) {
            for (int j = 0; j < state.action[i].size(); j++) {
                state.action[i][j]+=7;
            }
        }
    }
}

//Simple function for finding the jar across board
int across(int jar){ return 12 - jar; }

PlayGame::Node* PlayGame::result(Node& state, std::vector< int > action){
    //This is what generates children. This takes a state and an action
    //and creates a new_state as a child of the given state based upon
    //the action taken.
    children_generated++;
    std::unique_ptr< Node > new_state(new Node);
    //pull previous values from parent (and adjust as necessary)
    new_state->parent = &state;
    new_state->player_max = !state.player_max;
    new_state->depth = state.depth + 1;
    int side_marker = 7*!new_state->player_max;
    //copy the board
    for(int i = 0; i < 14; i++){
        new_state->board.push_back(state.board[i]);
    }
    //apply the action
    for(int i = 0; i < action.size(); i++){
        //add our result_of_play values for reference later
        new_state->result_of_play.push_back(action[i]);
        //pick up our stones from jar
        int stones = new_state->board[action[i]];
        new_state->board[action[i]] = 0;
        int cursor = action[i] + 1;
        //play the stones from jar
        while(stones > 0){
            //note we can ignore the case where we end in our own kalah
            //This is handled by the actions function
            if(cursor == (6 + side_marker)){
                //if the next space is the opponent's kalah
                cursor++;
                cursor%=14;
            } else if(stones == 1) {
                //last stone
                if(new_state->board[cursor] == 0 && new_state->board[across(cursor)] != 0
                   && cursor > side_marker && cursor < 6 + side_marker){
                    //if we are ending in an empty jar on our side and our opponent
                    //jar across the board isn't empty
                    new_state->board[6 + side_marker]+=new_state->board[across(cursor)];
                    new_state->board[6 + side_marker]++;
                    stones--;
                }
                new_state->board[cursor]++;
                stones--;
            } else {
                new_state->board[cursor]++;
                stones--;
                cursor++;
                cursor%=14;
            }
        }// while loop
    }// for loop
    //check to see if a board side is cleared and apply the board clear if so
    bool player_one_empty = true;
    bool player_two_empty = true;
    for(int i = 0; i < 6; i++){
        if(new_state->board[i] != 0) player_one_empty = false;
        if(new_state->board[i + 7] != 0) player_two_empty = false;
    }
    if(player_one_empty){
        for(int i = 7; i < 13; i++){
            new_state->board[6]+=new_state->board[i];
            new_state->board[i] = 0;
        }
    } else if(player_two_empty){
        for(int i = 0; i < 6; i++){
            new_state->board[13]+=new_state->board[i];
            new_state->board[i] = 0;
        }
    }
    Node* return_state = new_state.get();
    state.children.push_back(std::move(new_state));
    return return_state;
}

void PlayGame::actions_move_again(Node& state, std::vector< int > half_board, std::vector< int > current_action){
    //Helper function for void PlayGame::actions( . . . );
    half_board[current_action.back()] = 0;
    bool populated_board = false;
    for(int i = 0; i < 6; i++){
        if(half_board[i] != 0){
            populated_board = true;
            break;
        }
    }
    if(populated_board == false){
        state.action.push_back(current_action);
        return;
    }

    for(int i = current_action.back() + 1; i < 6; i++){
        half_board[i]++;
    }
    for(int i = 0; i < 6; i++){
        if(half_board[i] != 0){
            std::vector< int > current_subaction;
            for(int j = 0; j < current_action.size(); j++){
                current_subaction.push_back(current_action[j]);
            }
            current_subaction.push_back(i);
            if(half_board[i] + i == 6){
                actions_move_again(state, half_board, current_subaction);
                continue;
            }
            state.action.push_back(current_subaction);
        }
    }
}

/******************************************************************************
/  Heuristics and their helper functions
/*****************************************************************************/

double PlayGame::calculate_heuristic(Node& current_board, int selection){
    if(selection == 0){
        return alabandi_heuristic(current_board);
    }
    if(selection == 1){
        return bell_heuristic(current_board);
    }
    if(selection == 2){
        return coplin_heuristic(current_board);
    }
    return simple_heuristic(current_board);
}

static double scoreOpMove(std::vector<int> &board, int move, bool player1) {
    int marbles = board[move];
    int curr = move + 1;
    double scores = 0;
    double overflows = 0;
    bool multiMove = false;

    std::vector<int> temp_board (board);
    temp_board[move] = 0;

    if (player1) {
        while (marbles != 0) {
            if (curr != 13) { //don't put anything in opponents store
                if (curr < 6) {
                    //test for capture
                    if (marbles == 1 && temp_board[curr] == 0) {
                        scores++; //store the last marble

                        //store the opposing opponent pot
                        switch(curr) {
                            case 0: scores += temp_board[12];
                                break;
                            case 1: scores += temp_board[11];
                                break;
                            case 2: scores += temp_board[10];
                                break;
                            case 3: scores += temp_board[9];
                                break;
                            case 4: scores += temp_board[8];
                                break;
                            case 5: scores += temp_board[7];
                                break;
                        }
                    }
                    else {
                        //not a capture, but still our own pot
                        scores++;
                    }
                }
                else if (curr == 6) {
                    //scoring pot
                    scores++;

                    //test for multi move
                    if (marbles == 1) {
                        multiMove = true;
                    }
                }
                else if (curr > 6) {
                    //opponents pot
                    overflows++;
                }
            }
            curr = (curr + 1) % 14;
            marbles--;
        }

        if (multiMove) {
            scores *= 1.5;
        }

        return (scores - ((overflows * 0.3) * (overflows * 0.3)));
    }
    else { //player2
        while (marbles != 0) {
            if (curr != 6) { //don't put anything in opponents store
                if (curr == 13) {
                    //scoring pot
                    scores++;

                    //test for multi move
                    if (marbles == 1) {
                        multiMove = true;
                    }
                }
                else if (curr > 6) {
                    //test for capture
                    if (marbles == 1 && temp_board[curr] == 0) {
                        scores++; //store the last marble

                        //store the opposing opponent pot
                        switch(curr) {
                            case 7: scores += temp_board[5];
                                break;
                            case 8: scores += temp_board[4];
                                break;
                            case 9: scores += temp_board[3];
                                break;
                            case 10: scores += temp_board[2];
                                break;
                            case 11: scores += temp_board[1];
                                break;
                            case 12: scores += temp_board[0];
                                break;
                        }
                    }
                    else {
                        //not a capture, but still our own pot
                        scores++;
                    }
                }
                else if (curr < 6) {
                    //opponents pot
                    overflows++;
                }
            }
            curr = (curr + 1) % 14;
            marbles--;
        }

        if (multiMove) {
            scores *= 1.5;
        }

        return (scores - ((overflows * 0.3) * (overflows * 0.3)));
    }
}

double scoreMove(std::vector<int> &board, int move, bool player1) {
    int marbles = board[move];
    int curr = move + 1;
    double scores = 0;
    double overflows = 0;
    double moveScore = 0;
    double opScore = 0;
    bool multiMove = false;

    std::vector<int> temp_board (board);
    temp_board[move] = 0;

    if (player1) {
        while (marbles != 0) {
            if (curr != 13) { //don't put anything in opponents store
                if (curr < 6) {
                    //test for capture
                    if (marbles == 1 && temp_board[curr] == 0) {
                        scores++; //store the last marble

                        //store the opposing opponent pot
                        switch(curr) {
                            case 0: scores += temp_board[12];
                                temp_board[12] == 0;
                                break;
                            case 1: scores += temp_board[11];
                                temp_board[11] == 0;
                                break;
                            case 2: scores += temp_board[10];
                                temp_board[10] == 0;
                                break;
                            case 3: scores += temp_board[9];
                                temp_board[9] == 0;
                                break;
                            case 4: scores += temp_board[8];
                                temp_board[8] == 0;
                                break;
                            case 5: scores += temp_board[7];
                                temp_board[7] == 0;
                                break;
                        }
                    }
                    else {
                        //not a capture, but still our own pot
                        scores++;
                    }
                }
                else if (curr == 6) {
                    //scoring pot
                    scores++;

                    //test for multi move
                    if (marbles == 1) {
                        multiMove = true;
                    }
                }
                else if (curr > 6) {
                    //opponents pot
                    overflows++;
                }
            }
            curr = (curr + 1) % 14;
            marbles--;
        }

        if (multiMove) {
            scores *= 1.5;
        }

        moveScore = scores - ((overflows * 0.3) * (overflows * 0.3));

        //score how this move sets up the opponent
        double maxOpScore = 0;
        for (int i = 7; i < 13; ++i) {
            //score this move
            opScore = scoreOpMove(temp_board, i, false);

            if (opScore > maxOpScore) {
                maxOpScore = opScore;
            }
        }

        if (maxOpScore == 0) {
            return moveScore;
        }

        return moveScore / maxOpScore;
    }
    else { //player2
        while (marbles != 0) {
            if (curr != 6) { //don't put anything in opponents store
                if (curr == 13) {
                    //scoring pot
                    scores++;

                    //test for multi move
                    if (marbles == 1) {
                        multiMove = true;
                    }
                }
                else if (curr > 6) {
                    //test for capture
                    if (marbles == 1 && temp_board[curr] == 0) {
                        scores++; //store the last marble

                        //store the opposing opponent pot
                        switch(curr) {
                            case 7: scores += temp_board[5];
                                temp_board[5] == 0;
                                break;
                            case 8: scores += temp_board[4];
                                temp_board[4] == 0;
                                break;
                            case 9: scores += temp_board[3];
                                temp_board[3] == 0;
                                break;
                            case 10: scores += temp_board[2];
                                temp_board[2] == 0;
                                break;
                            case 11: scores += temp_board[1];
                                temp_board[1] == 0;
                                break;
                            case 12: scores += temp_board[0];
                                temp_board[0] == 0;
                                break;
                        }
                    }
                    else {
                        //not a capture, but still our own pot
                        scores++;
                    }
                }

                else if (curr < 6) {
                    //opponents pot
                    overflows++;
                }
            }
            curr = (curr + 1) % 14;
            marbles--;
        }

        if (multiMove) {
            scores *= 1.5;
        }

        moveScore = scores - ((overflows * 0.3) * (overflows * 0.3));

        double maxOpScore = 0;
        for (int i = 0; i < 6; ++i) {
            //score this move
            opScore = scoreOpMove(temp_board, i, true);

            if (opScore > maxOpScore) {
                maxOpScore = opScore;
            }
        }

        if (maxOpScore == 0) {
            return moveScore;
        }

        return moveScore / maxOpScore;
    }
}

double PlayGame::alabandi_heuristic(Node& state)
{
    int player = 2 - state.player_max;
    int i;
    int score;

    // Kalah counts 6 times more, but stones count too.
    if(player == 2)
    {

        score = 6 * ( state.board[6] - state.board[13] );


        for ( i = 0; i <= 5; i++ )
            score += state.board[i];

        for ( i = 7; i <= 12; i++ )
            score -= state.board[i];
    }
    else if (player == 1)
    {
        score = 6 * ( state.board[13] - state.board[6] );

        for ( i = 0; i <= 5; i++ )
            score -= state.board[i];

        for ( i = 7; i <= 12; i++ )
            score += state.board[i];
    }

    return score;
}

double PlayGame::bell_heuristic(Node& state){
    //First calculate the difference in scores
    //Then add a fifth the number of seeds that are in jars which
    //can't play into opponent's jars.
    double score = state.board[6] - state.board[13];
    double coeff = .2;
    for(int i = 0; i < 6; i++){
        if(state.board[i] < 6 - i) score+= coeff * (state.board[i]);
    }
    for(int i = 7; i < 13; i++){
        if(state.board[i] < 13 - i) score-= coeff * (state.board[i]);
    }
    return score;
}

double PlayGame::coplin_heuristic(Node& inNode) {
    //evaluate move
    double score = 0;
    double maxScore = 0;

    if (inNode.player_max) {
        for (int i = 0; i < 6; ++i) {
            //score this move
            score = scoreMove(inNode.board, i, true);

            if (score > maxScore) {
                maxScore = score;
            }
        }
    }
    else {
        for (int i = 7; i < 13; ++i) {
            //score this move
            score = scoreMove(inNode.board, i, false);

            if (score > maxScore) {
                maxScore = score;
            }
        }
    }

    return maxScore;
}

double PlayGame::simple_heuristic(Node& state){
    return state.board[6] - state.board[13];
}

/******************************************************************************
/  Misc. Functions
/*****************************************************************************/

void PlayGame::output_path(){
    for(int i = 0; i < path.size(); i++){
        for(int j = 0; j < path[i].size(); j++){
            std::cout << path[i][j] << " ";
        }
        if(i + 1 != path.size()) std::cout << "--> ";
    }
}

void PlayGame::output_path(std::ofstream& fout){
    for(int i = 0; i < path.size(); i++){
        for(int j = 0; j < path[i].size(); j++){
            fout << path[i][j] << " ";
        }
        if(i + 1 != path.size()) fout << "--> ";
    }
}