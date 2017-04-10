#ifndef PACKET_H
#define PACKET_H

#include <connection.h>
#include <cstdint>
#include <string>

using std::string;

#define UPLOAD      0
#define DOWNLOAD    1
#define ADDLIST     2
#define PLAYPAUSE   3
#define FASTFORWORD 4
#define REWIND      5
#define SKIPTRACK   6
#define STREAM      7
#define HEADER      8
#define PROGRESS    9
#define AVAILSONG     10
#define PLAYLIST      11

#define BUFFERSIZE 20000

#endif
