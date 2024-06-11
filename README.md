

To run the program, please follow the following steps:
1. In the sim subdirectory, please start the simulator: *simserver1*. Thereby, the simconfig.xml should use the Labyrinth file in the Wall folder. Of course other wall structures can be used as well. 
2. In the sim subdirectory, please start the ulmsserver: *ulmsserver*
3. In the sim subdirectory, please start the MRC:  *mrc -s8000 -t1*
4. As the last step, the program should start. Therefore, first please type the following command where the Labyrinth.c file is stored: *gcc -0 Labyrinth Labyrinth.c 'xml2-config --cflags --libs'*. 
   Following the program can be started: *./Labyrinth*. 
