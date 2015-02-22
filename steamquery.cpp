#include "steamquery.h"

SteamQuery::SteamQuery(const std::string &_hostname, const std::string &_port)
    :hostname(_hostname), port(_port) {}

int SteamQuery::send_query()
{
    int status;
    struct addrinfo hints, *servinfo;
    struct timeval tv;
    memset(&hints, 0, sizeof hints);

    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    if ((status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n\n";
        return 1;
    }

    socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    connect(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);

    byte request[] = {0xff, 0xff, 0xff, 0xff, 0x54, 0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x20, 0x45, 0x6e, 0x67, 0x69, 0x6e, 0x65, 0x20, 0x51, 0x75, 0x65, 0x72, 0x79, 0x00};

    send(socketfd, request, sizeof request, 0);
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    return 0;
}

int SteamQuery::listen()
{
    byte buffer[1024];
    int recvsz = recv(socketfd, buffer, sizeof buffer, 0);

    if (recvsz == EWOULDBLOCK || recvsz == EAGAIN || recvsz < 1)
        return 1;

    std::cout << "success, got " << recvsz << " bytes \n";
    parse_response(std::string(buffer, buffer + recvsz));

    return 0;
}

void SteamQuery::parse_response(std::string raw)
{
    raw = raw.substr(4);

    server.header   = raw[0];
    server.protocol = raw[1];
    raw = raw.substr(2);

    server.name = raw.substr(0, raw.find(std::string("\x00", 1)));
    raw = raw.substr(raw.find(std::string("\x00", 1)) + 1);

    server.map = raw.substr(0, raw.find(std::string("\x00", 1)));
    raw = raw.substr(raw.find(std::string("\x00", 1)) + 1);

    server.folder = raw.substr(0, raw.find(std::string("\x00", 1)));
    raw = raw.substr(raw.find(std::string("\x00", 1)) + 1);

    server.game = raw.substr(0, raw.find(std::string("\x00", 1)));
    raw = raw.substr(raw.find(std::string("\x00", 1)) + 1);

    server.id = 0;
    raw = raw.substr(2);

    server.current_players = raw[0];
    server.max_players     = raw[1];
    server.bots            = raw[2];
    server.environment     = raw[3];
    server.visibility      = raw[4];
    server.vac             = raw[5];
    raw = raw.substr(6);

    server.version = raw.substr(0, raw.find(std::string("\x00", 1)) + 1);
    raw = raw.substr(raw.find(std::string("\x00", 1)) + 1);

    server.edf = raw[0];
}
