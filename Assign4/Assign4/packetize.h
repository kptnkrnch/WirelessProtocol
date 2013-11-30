#ifndef PACKETIZE_H
#define PACKETIZE_H

#include <fstream>

#include "global.h"

#define SEED 0xff

#define PACKET_SIZE 1024
#define DATA_SIZE PACKET_SIZE - 4

bool recievePacket(char[]);
void readFile(std::fstream&);
void packetize(char[]);

#endif
