To compile on a unix terminal use

	gcc -std=c++11 main.cpp PlayGame.cpp -o kalah

To use program:

./kalah max_alg max_heu min_alg min_heu max_depth min_depth diagfile.csv

where:

     max_alg: Is the algorithm player 1 is using. Use 0 for 
              Rich/Knight, 1 for Russell/Norvig
     max_heu: The heuristic player 1 is using. 0 for Ghadeer's, 1 
              for Chris's, 2 for Coplin's, and 3 for a simple 
              heuristic comparing kalah values
     min_alg: Analogous to max_alg, but for player 2
     min_heu: Analogous to max_alg, but for player 2
     max_depth: The maximum size of player 1's tree
     min_depth: The maximum size of player 2's tree
     
diagfile.csv: creates a file where the program's working directory
              which records data for use.

Console output is rather lengthy, I recommend you redirect your output
to a file.

ALSO, running past 10 depth can cause problems (the graph gets HUGE),
I recommend staying 8 or below.
