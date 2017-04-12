#ifndef GLOBAL_H
#define GLOBAL_H

#include <connection.h>
#include <cstdint>
#include <string>

using std::string;

#define DOWNLOAD        1
#define ADDLIST         2
#define PLAYPAUSE       3
#define FASTFORWORD     4
#define REWIND          5
#define SKIPTRACK       6
#define STREAM          7
#define HEADER          8
#define PROGRESS        9
#define AVAILSONG       10
#define PLAYLIST        11
#define COMPLETE        12
#define UPLOAD          13

#define PEER_REQUEST    21
#define PEER_ACCEPT     22
#define PEER_REJECT     23
#define PEER_DISCONNECT 24
#define PEER_STREAM     25

#define BUFFERSIZE      20000
#define SERVERPORT      7000
#define PEERPORT        9000
#define MULTICASTSERVER "234.57.7.8"
#endif
