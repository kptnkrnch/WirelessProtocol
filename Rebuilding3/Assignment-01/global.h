#ifndef GLOBAL_H
#define GLOBAL_H

#include <Windows.h>

#define NUL  0x00
#define EOT  0x04
#define ENQ  0x05
#define ACK  0x06
#define SOT1 0x11
#define SOT2 0x12
#define NAK  0x15
#define SYN  0x16


typedef struct Globals {

	HANDLE* hSem;
	HANDLE* hComm;
	OVERLAPPED ov;
} Globals;

#endif