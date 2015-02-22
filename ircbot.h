#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <vector>
#include <fstream>


class IrcBot
{

private:
    std::string hostname, port, nick, chans, file;
    int socketfd;

public:
    IrcBot(const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);
    int connect_to_server();
    void send_raw(std::string msg);
    void send_msg(const std::string &, const std::string);
    int listen();
    void message_handler(const std::string &, const std::string &, const std::string &);

};
