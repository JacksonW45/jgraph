// Name: Jackson Wolan
// Netid: jwolan1
// Date: 3/1/2026

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cstdlib>

#include "json.hpp"

// Set namespaces, we'll be using JSON
using namespace std;
using json = nlohmann::json;

/* Function takes a URL as string input and gives the longest
   string of digits available. This is our gameID for the ESPN 
   game of the users choice.
*/
string getGameId(string input)
{
    string current = "";
    string best = "";

    for (int i = 0; i < (int)input.length(); i++)
    {
        if (isdigit((unsigned char)input[i]))
        {
            current += input[i];
            if (current.length() > best.length())
                best = current;
        }
        else
        {
            current = "";
        }
    }

    return best;
}

// limits values (v) to set ranges (lo, hi)
// **used to keep shot positions in bounds
double clamp(double v, double lo, double hi)
{
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

int main(int argc, char** argv)
{
    // Declare vars, kind of just in the order I needed them
    bool onlyDigits;
    string input, gameId;
    
    char buffer[4096];
    int code;
    FILE* pipe;
    string url, cmd;
    string content = "";

    json j, teams;
    string id, ha;
    string homeId = "";
    string awayId = "";

    // Check inputs
    if (argc < 2) {
        cout << "make GAME=<espn_url>\n";
        return -1;
    }

    // First we need the game ID from the user input
    input = argv[1];

    // checks if we got just the gameID or the full URL
    onlyDigits = true;
    for (int i = 0; i < (int)input.length(); i++) {
        if (!isdigit((unsigned char)input[i])) {
            onlyDigits = false;
            break;
        }
    }

    // if we have just digits set gameId to input
    if (onlyDigits) {
        gameId = input;
    }
    // Otherwise we call getGameId() to get the digits
    else {
        gameId = getGameId(input);
    }
    
    // Check that we pull a valid gameId
    if (gameId.length() < 6) {
        fprintf(stderr, "Failed to pull a valid gameId");
        return -1;
    }

    // Use court.jgr to lay the court down (constant)
    ifstream courtIn("src/court.jgr");
    if (!courtIn)
    {
        cout << "Could not open court.jgr (must be in the folder)\n";
        return -1;
    }
    // Reads the file into the string courtIn
    string courtText((istreambuf_iterator<char>(courtIn)), istreambuf_iterator<char>());
    courtIn.close();

    // Goes and fetches the json api from ESPN using the gameId from earlier
    url = "https://site.api.espn.com/apis/site/v2/sports/basketball/mens-college-basketball/summary?event=" + gameId;
    
    // Format the url as a curl command and pipe the command to terminal
    cmd = "curl -s \"" + url + "\"";
    pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        cout << "curl failed.\n";
        return -1;
    }

    //  Read in data from the buffer into a string
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        content += buffer;
    }

    // Make sure pipe closed properly
    code = pclose(pipe);
    if (code != 0)
    {
        cout << "curl returned error.\n";
        return -1;
    }

    // Now we need to pull the shot data from the json file
    j = json::parse(content);

    // Pull specifically the boxscore and which teams it is
    teams = j["boxscore"]["teams"];
    for (int i = 0; i < (int)teams.size(); i++) {
        ha = teams[i]["homeAway"];
        id = teams[i]["team"]["id"];
        
        // save home and away team id's for later
        if (ha == "home") {
            homeId = id;
        }
        else if (ha == "away") {
            awayId = id;
        }   
    }

    // Check we successfully pulled both
    if (homeId == "" || awayId == "")
    {
        cout << "Home/away team IDs missing.\n";
        return -1;
    }

    // Create two .jgr files, one for home one for away
    ofstream homeOut("home.jgr");
    ofstream awayOut("away.jgr");

    // Double check the files exist
    if (!homeOut || !awayOut)
    {
        cout << "Could not create home.jgr / away.jgr\n";
        return -1;
    }

    // Add the jgraph court to the output
    homeOut << courtText << "\n";
    awayOut << courtText << "\n";

    /* SETTINGS:
        These numbers adjust how big our X's and O's are
        in the final ouput
    */
    double madeSize = 0.35; // circle size
    double missD = 0.35;    // half-diagonal of the X

    int homeCount = 0;
    int awayCount = 0;

    bool made;
    double x,y;
    double r = 0, g = 0, b = 0;
    string teamId;
    ofstream* outFile = nullptr;


    // ----- loop plays and write overlays -----
    json plays = j["plays"];

    for (int i = 0; i < (int)plays.size(); i++)
    {
        // Make sure the play at hand is a shot
        if (!plays[i].contains("shootingPlay")) continue;
        if (!plays[i]["shootingPlay"]) continue;

        if (!plays[i].contains("coordinate")) continue;
        if (!plays[i]["coordinate"].contains("x")) continue;
        if (!plays[i]["coordinate"].contains("y")) continue;

        if (!plays[i].contains("team")) continue;
        if (!plays[i]["team"].contains("id")) continue;

        // Get the id of the team who attempted the shot
        teamId = plays[i]["team"]["id"];
        
        // Get shot position coordinates
        x = (double)plays[i]["coordinate"]["x"];
        y = (double)plays[i]["coordinate"]["y"];

        // clamp into your half-court bounds
        x = clamp(x, 0.0, 50.0);
        y = clamp(y, 0.0, 47.0);

        // Find out if the shot was made or missed
        made = false;
        if (plays[i].contains("scoringPlay")) {
            made = plays[i]["scoringPlay"];
        }

        // Get the ptr to the correct output file
        outFile = nullptr;
        if (teamId == homeId){
            outFile = &homeOut;
        }
        else if (teamId == awayId) {
            outFile = &awayOut;
        }
        // Skip incase id is bad
        else continue;

        // Choose team colors (edit if you want)
        if (teamId == homeId) { r = 0; g = 0; b = 1; } // home = blue
        else { r = 1; g = 0; b = 0; }                  // away = red

        // Output O if made at the position
        if (made)
        {
            // filled circle
            (*outFile) << "newcurve linetype none\n";
            (*outFile) << "  color " << r << " " << g << " " << b << "\n";
            (*outFile) << "  marktype circle\n";
            (*outFile) << "  marksize " << madeSize << "\n";
            (*outFile) << "  fill 1\n";
            (*outFile) << "  pts\n";
            (*outFile) << "    " << x << " " << y << "\n\n";
        }
        // Output X if missed at the position
        else
        {
            // X (two line segments)
            (*outFile) << "newcurve linetype solid marktype none\n";
            (*outFile) << "  color " << r << " " << g << " " << b << "\n";
            (*outFile) << "  pts\n";
            (*outFile) << "    " << (x - missD) << " " << (y - missD) << "\n";
            (*outFile) << "    " << (x + missD) << " " << (y + missD) << "\n\n";

            (*outFile) << "newcurve linetype solid marktype none\n";
            (*outFile) << "  color " << r << " " << g << " " << b << "\n";
            (*outFile) << "  pts\n";
            (*outFile) << "    " << (x - missD) << " " << (y + missD) << "\n";
            (*outFile) << "    " << (x + missD) << " " << (y - missD) << "\n\n";
        }

        if (teamId == homeId) {
            homeCount++;
        }
        if (teamId == awayId) {
             awayCount++;
        }
    }


    homeOut.close();
    awayOut.close();

    return 0;
}