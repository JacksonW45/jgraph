# JGraph - CS494 Lab

This project takes a link to a ESPN NCAA basketball game and ouputs a shotgraph for both the home and away teams over the course of the game.

## Build and Run
Clone the respository and run:
make

make on its own will compile with 5 NCAA games, each pdf is converted and put into a pdf folder.

The jgraph.cpp can also be executed as:
./jgraph "<espn_url>"   or 
./jgraph <espn_gameId>

Executing ./jgraph 401808271 will output home.pdf and away.pdf which shows the shotgraph of the recent Alabama Crimson Tide and Tennessee Volunteers basketball game.