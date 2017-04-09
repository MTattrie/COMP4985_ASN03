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

#define BUFFERSIZE 1024

#endif
