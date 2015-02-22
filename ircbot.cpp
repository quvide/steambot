#include "ircbot.h"
#include "steamquery.h"

IrcBot::IrcBot(const std::string &_hostname, const std::string &_port, const std::string &_nick, const std::string &_chans, const std::string &_file)
    :hostname(_hostname), port(_port), nick(_nick), chans(_chans), file(_file) {}

int IrcBot::connect_to_server()
{
    std::cout << "\n" << hostname << "\n" << port << "\n" << nick << "\n\nconnecting...\n\n";

    int status;
    struct addrinfo hints, *servinfo;
    memset(&hints, 0, sizeof hints);

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n\n";
        return 1;
    }

    socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    connect(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);

    send_raw("USER " + nick + " 0 * :" + nick);
    send_raw("NICK " + nick);

    return 0;
}

void IrcBot::send_raw(std::string msg)
{
    msg = msg + "\r\n";
    std::cout << ">> " << msg;
    std::cout << "successfully sent " << send(socketfd, msg.c_str(), strlen(msg.c_str()), 0) << " bytes\n";
}

void IrcBot::send_msg(const std::string &chan, const std::string msg)
{
    send_raw("PRIVMSG " + chan + " :" + msg);
}

int IrcBot::listen()
{
    char buffer[1024];
    for (;;)
    {
        int recvsz = recv(socketfd, buffer, sizeof buffer, 0);
        std::string msg = std::string(buffer);

        std::vector<std::string> lines;
        std::string separator = "\r\n";

        int pos = msg.find(separator);
        while (pos != std::string::npos) {
            if (pos > 0)
                lines.push_back(msg.substr(0, pos + 2));

            msg = msg.substr(pos + 2);
            pos = msg.find(separator);
        }

        if (msg.length() > 0)
            lines.push_back(msg);

        for (int i = 0; i < lines.size(); i++) {
            std::cout << lines[i];

            lines[i] = lines[i].substr(0, lines[i].find("\r\n"));

            if (lines[i].find(":") == 0) {
                std::string nick, host, cmd, target, data;
                lines[i] = lines[i].substr(1);

                if (lines[i].substr(1, lines[i].find(" ") - 1).find("!") != std::string::npos) {
                    nick = lines[i].substr(0, lines[i].find("!"));
                    lines[i] = lines[i].substr(lines[i].find("!") + 1);
                    host = lines[i].substr(0, lines[i].find(" "));
                } else {
                    host = lines[i].substr(0, lines[i].find(" "));
                }

                lines[i] = lines[i].substr(lines[i].find(" ") + 1);

                cmd = lines[i].substr(0, lines[i].find(" "));
                lines[i] = lines[i].substr(lines[i].find(" ") + 1);

                if (lines[i].find(" ") != std::string::npos) {
                    target = lines[i].substr(0, lines[i].find(" "));
                    lines[i] = lines[i].substr(lines[i].find(" ") + 1);
                    if (lines[i].find(":") == 0) {
                        data = lines[i].substr(1);
                    } else {
                        data = lines[i];
                    }
                } else {
                    target = lines[i];
                }



                if (cmd == "001") {
                    send_raw("JOIN " + chans);
                } else if (cmd == "PRIVMSG") {
                    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
                    if (target.find("#") == 0)
                        message_handler(target, nick, data);
                    else
                        message_handler(nick, nick, data);
                }

                //std::cout << nick << "/" << host << "/" << cmd << "/" << target << "/" << data << "\n";

            } else if (lines[i].find("PING") == 0) {
                std::string payload = lines[i].substr(4);
                send_raw("PONG" + payload);
            }
        }

        memset(&buffer, 0, sizeof buffer);
    }

    return 0;
}

void IrcBot::message_handler(const std::string &chan, const std::string &nick, const std::string &msg)
{
    if (msg.find("!") != 0)
        return;

    std::string cmd, data;
    cmd = msg.substr(1, msg.find(" ") - 1);
    if (msg.find(" ") != std::string::npos)
        data = msg.substr(msg.find(" ") + 1);

    if (cmd == "p") {
        if (data.empty())
            data = "default";

        std::ifstream server_file;
        server_file.open(file.c_str());
        std::string line;

        bool found;

        while (std::getline(server_file, line)) {
            std::string nickname, hostname, port;

            nickname = line.substr(0, line.find(":"));
            line = line.substr(line.find(":") + 1);

            hostname = line.substr(0, line.find(":"));
            line = line.substr(line.find(":") + 1);

            port = line;



            if (nickname == data) {
                found = true;
                std::cout << "matching nickname for server \"" << data << "\", sending query to " << hostname << ":" << port << "\n";
                SteamQuery steamquery(hostname, port);
                if (steamquery.send_query() == 0 && steamquery.listen() == 0) {
                    send_msg(chan, std::to_string((int)steamquery.server.current_players) + "/" + std::to_string((int)steamquery.server.max_players) + " on map " + steamquery.server.map + " (" + steamquery.server.game + ") " + steamquery.server.name);
                }
            }
        }
        server_file.close();

        if (!found)
            send_msg(chan, nick + ": server was not found.");

    }/* else if (cmd == "ping") {
        send_msg(chan, "pong! " + nick);
    }*/
}
