#ifndef PACKETIZE_H
#define PACKETIZE_H

#include <fstream>

#define NUL  0x00
#define SOT1 0x11
#define SOT2 0x12
#define SYN  0x16

#define SEED 0xff

#define PACKET_SIZE 1024
#define DATA_SIZE PACKET_SIZE - 4

bool recievePacket(char[]);
void readFile(std::fstream&);
void packetize(char[]);

#endif
