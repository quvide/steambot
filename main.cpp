#include <iostream>
#include <getopt.h>

#include "ircbot.h"

void print_usage()
{
    std::cerr << "\n" <<
                 "Usage: steambot\n" <<
                 "           -h --hostname irc ip\n" <<
                 "           -p --port irc port\n" <<
                 "           -n --nick irc nick\n" <<
                 "           -c --chan irc channel(s) separated by commas\n" <<
                 "           -s --servers servers file\n" <<
                 "\n" <<
                 "Example: ./steambot -h irc.quakenet.org -p 6667 -n steambot -c #channel1,#channel2 -s servers.cfg\n\n";
}

int main(int argc, char *argv[])
{

    int opt = 0;

    static struct option long_options[] = {
        {"hostname",    required_argument, 0, 'h'},
        {"port",        required_argument, 0, 'p'},
        {"nick",        required_argument, 0, 'n'},
        {"chan",        required_argument, 0, 'c'},
        {"servers",     required_argument, 0, 's'}
    };

    std::string hostname, port, nick, chan, file;

    while ((opt = getopt_long(argc, argv, "h:p:n:c:s:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                hostname = std::string(optarg);
                break;
            case 'p':
                port = std::string(optarg);
                break;
            case 'n':
                nick = std::string(optarg);
                break;
            case 'c':
                chan = std::string(optarg);
                break;
            case 's':
                file = std::string(optarg);
                break;
            default:
                print_usage();
                return 1;
        }
    }

    if (hostname.empty() || port.empty() || nick.empty() || chan.empty() || file.empty()) {
        print_usage();
        return 1;
    }

    IrcBot ircbot(hostname, port, nick, chan, file);

    int ret = ircbot.connect_to_server();

    if (ret == 0)
        ircbot.listen();

    return ret;
}
