#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

class SteamQuery
{

private:
    std::string hostname, port;
    int socketfd;

public:
    typedef unsigned char byte;

    struct steam_response {
        byte header;
        byte protocol;
        std::string name;
        std::string map;
        std::string folder;
        std::string game;
        short id;
        byte current_players;
        byte max_players;
        byte bots;
        byte server_type;
        byte environment;
        byte visibility;
        byte vac;
        std::string version;
        byte edf;
        struct edf_data {
            short port;
            long long steamid;
            short sourcetv_port;
            std::string sourcetv_name;
            std::string keywords;
            long long gameid;
        } edf_data;
    } server;

    SteamQuery(const std::string &, const std::string &);

    int send_query();
    int listen();
    void parse_response(std::string);
};
