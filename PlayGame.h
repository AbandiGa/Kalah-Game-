//
// Created by Chris on 4/21/2017.
//

#ifndef TERMINALAPP_PLAYGAME_H
#define TERMINALAPP_PLAYGAME_H
#include <memory>
#include <vector>
#include <limits>

class PlayGame{
public:
    //Constructor, call this to make a move.
    //To retrieve the move's new board, examine PlayGame.next_moves_board
    //To retrieve the move's value, examine PlayGame.heuristic_score
    //To retrieve the move's path, examine PlayGame.path
    //    Board: vector of size 14 representing the current board state
    //algorithm: 0 for Rich + Knight, 1 for Norvig and Russell
    //   player: 0 for min's turn, 1 for max's turn
    //heuristic: -1 for current score, 0 for alabandi, 1 for bell, 2 for coplin
    PlayGame(std::vector<int>& board, bool algorithm, bool player, int heuristic, int max_depth);

    struct Node{
        /* Connectors */
        std::vector< std::unique_ptr< Node >> children;
        std::vector< double > children_value;
        Node* parent;

        /* Data */
        std::vector< int > board; //board for the current node
        int depth; //depth of the current node
        bool player_max; //max is player 1, false => player 2 (min)
        std::vector< int > result_of_play; //this will save the actions by the parent to get to here
        std::vector< std::vector< int > > action; //all possible actions that can be made from this board
        int selected; //index to the selected action from above
        double heuristic_value; //used in Rich&Knight for keeping value on node

        //Node constructor
        Node() : children(0), children_value(0), board(0), result_of_play(0), action(0){
            parent == nullptr;
            depth = 0;
            player_max = 0;
            selected = 0;
            heuristic_value = 0;
        }
    };

    /* Tree Data */

    std::unique_ptr< Node > root; //starting node
    int max_depth; //maximum depth of the tree
    std::vector< int > move; //integers corresponding to the next move
    std::vector< std::vector< int >> path; //the path of predicted moves
    int children_generated; //Number of nodes made overall (root inclusive)
    int function_used; //0 for Ghadeer's, 1 for Chris's, 2 for Jared's, other for simple dif of score
    double heuristic_score; //score of the move based upon the heuristic used
    std::vector< int > next_moves_board; //board after playing the found move

    /* Functions */
    /*
     * alpha-beta-search from Luger
     */
    std::vector< int > alpha_beta_search(Node&); //Norvig and Luger's algorithm
    double max_value(Node& state, double alpha, double beta);
    double min_value(Node& state, double alpha, double beta);
    bool cutoff_test(Node& state);


    /*
     * minimax_a_b from Rich and Knight
     */
    //Note, the return values for this function are embedded in the nodes
    void minimax_a_b(Node&, double, double); //Rich and Knight's algorithm


    /*
     * Tree Operations
     */
    void generate_children(Node&);
    void generate_child(Node &state, int i);
    void actions(Node&);
    Node* result(Node&, std::vector< int >);
    void actions_move_again(Node& state, std::vector<int> half_board, std::vector<int> currentAction);


    /*
     *  Heuristic and helpers
     */
    double calculate_heuristic(Node&, int); //heuristic handler, calls the correct heuristic
    double alabandi_heuristic(Node&);
    double bell_heuristic(Node&);
    double coplin_heuristic(Node&);
    double simple_heuristic(Node&); //Simply returns the difference of the kalahs

    /*
     * Misc.
     */
    void output_path(); //used to output move's path to console
    void output_path(std::ofstream&); //used to output move's path to file
};

//helper function for end with rich & knight minimax
bool terminal_board(std::vector< int >);

#endif //TERMINALAPP_PLAYGAME_H
