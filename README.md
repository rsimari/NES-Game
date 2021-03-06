BERT's Adventure

bdalgarn, ebrady5, rsimari, tgayle

For our project, we were inspired by the speedrunning communities that have recently popped up for video games, especially those from older systems such as the Sega Genesis. So, we decided to make a game that would take the aspects of speed and competition and fit them within the constrained environment of NES development. We worked with a bit of assembly code for things like configuration, and coded our main game file in C before sending it to the cc65 compiler. Developing in this environment was challenging, due in part to hurdles like limited memory, learning how the NES PPU draws to the screen, limited color palette for sprites, and the need to use  global variables almost exclusively. Debugging was difficult as well, as the NES hardware doesn't throw simple error messages like the UNIX terminal does. On the technical side, completing this project gave us a new appreciation for the developers who grinded out long hours coding the games that we still enjoy playing today.

The aim of the game is to navigate the various mazes sequentially, and as quickly as possible -- the timer is always visible in the top-right corner of the screen, and is always ticking upward. Upon completion of the final stage, the player is notified of his or her final time, which can be used for bragging rights among friends or as motivation to improve. Various elements were added to the game to make it closer to what we felt made up the spirit of a "speedrun game." For example, doors in one stage must be entered twice to proceed, whereas they must be entered sequentially in others. In the final stage, doors warp the player around the map and closer to the goal. By playing the game repeatedly, players can memorize these layouts and object behaviors, improving their speed and completion time. We had a good time competing among ourselves to see who could get the best time, just like speedrunners compete for records in classic games.

Instructions:
1. Download the Wine Emulator: https://wiki.winehq.org/Download#source
2. Go to https://github.com/rsimari/NES-Game and git clone the repository in your terminal
3. Run "make" to compile all the important files
4. Run "make play" to run the game on the Emulator
5. Play the Game (works best with an NES controller)
6. Exit and run "make clean"
