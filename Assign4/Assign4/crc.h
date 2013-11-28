#ifndef CRC_H
#define CRC_H

#include <cstdlib>

unsigned short CRCCCITT(unsigned char *data, size_t length, unsigned short seed, unsigned short final);

#endif
