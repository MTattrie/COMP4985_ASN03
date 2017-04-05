#ifndef PACKET_H
#define PACKET_H

#include <cstdint>
#include <string>

using std::string;

enum class Command {
    RECV,
    SEND,
    PLAY,
    PAUSE,
    ADD
};


struct CommandPacket{
    Command command;
    string song;
    int size;
};


#endif
