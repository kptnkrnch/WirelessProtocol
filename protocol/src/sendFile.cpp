/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: sendFile.cpp
--
-- PROGRAM: 
--
-- FUNCTIONS: DWORD WINAPI sendBufferThread(LPVOID n);
--			  void send_packets(Globals*);
--			  bool enquire_line(Globals*);
--			  bool transmit_packet(Globals*, const char*);
--			  bool wait_for_acknowledgement(Globals*);
--			  void sendControlChar(HANDLE& hComm, char);
--
-- DATE: 11.21.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
#include <Windows.h>
#include <iostream>
#include "sendFile.h"
#include "global.h"
#include "Terminal.h"

using namespace std;

DWORD  dwBytesWritten;
Buffer buffer;

extern OVERLAPPED ov;
extern HANDLE semm;
extern Stats stats;
extern int send_control;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendBufferThread
--
-- DATE: 11.29.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: DWORD WINAPI sendBufferThread(LPVOID n);
--
-- RETURNS:
--
-- NOTES: Continuously checks buffer for packets to send
--
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI sendBufferThread(LPVOID n)
{

	Globals* global = (Globals*)n;
	dwBytesWritten = 0;
	*(global->hSem) = CreateSemaphore(NULL, 0, 1, NULL);

	//infinitely wait for the buffer to have at least one packet.
	while (!buffer.is_empty()) 
	{
		send_packets(global);
		Sleep(600 + (rand() % 600));
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: send_file (old)
--           send_packets
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.23.13 - replaced leftover pseudocode
--            11.26.13 - added semaphores and error checking
--            11.27.13 - added buffer and fix ENQ/EOT sending
--            11.28.13 - put in while loop
--            11.30.13 - change name to send_packets and interface; implement global
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: void send_file(char* data[]); (old)
--            void send_packets(Globals *global)
--
-- RETURNS: void.
--
-- NOTES: send_file() will constantly run at the start of program. It constantly checks the buffer for packets to
--        send. A maximum of 5 packets at a time will be sent before giving up the line to the reciever to transmit
--        data. As packets are transmited, they are removed from the buffer. 
-- 
----------------------------------------------------------------------------------------------------------------------*/

void send_packets(Globals *global)
{
	int packets_sent = 0;
	int packet_failure = 0;

	// if starting to send data, enquire line
	if(!enquire_line(global)) 
	{
		cerr << "could not enquire line" << endl;
		return;
	}

	// loop through all the data 
	while(!buffer.is_empty() && packets_sent < 5)
	{
		// transmit a packet
		if (!transmit_packet(global, buffer.get_packet()))
		{
			//MessageBox(NULL, TEXT("ERROR SENDING"), TEXT(""), MB_OK);
			packet_failure++;
			cerr << "error sending packet" << endl;
			if (packet_failure >= 5) {
				return;
			}
		}else
		{   // if packet sent success, increment packets send and update stats
			//MessageBox(NULL, TEXT("Packet Sent"), TEXT(""), MB_OK);
			send_control = send_control == SOT1 ? SOT2 : SOT1;
			packets_sent++;
			stats.totalPacketsSent_++;
			UpdateStats();
			packet_failure = 0;
			buffer.remove_packet();
		}
	}

	// send end of transmition
	//MessageBox(NULL, TEXT("Sending EOT"), TEXT(""), MB_OK);
	sendControlChar(global->hComm, EOT);
	//ReleaseSemaphore(*(global->hSem), 1, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: enquire_line
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.27.13 - added ENQ function
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool enquire_line() (old)
--            bool enquire_line(Globals *globals)
--
-- RETURNS: bool.
--
-- NOTES: transmits and enquiry for the line and waits for a response
-- 
----------------------------------------------------------------------------------------------------------------------*/

bool enquire_line(Globals *globals)
{
	// send enq
	//MessageBox(NULL, TEXT("Sending ENQ"), TEXT(""), MB_OK);
	sendControlChar(globals->hComm, ENQ);
	send_control = SOT1;

	// wait for reciever to acknowledge line is free
	return wait_for_acknowledgement(globals);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: transmit_packet
--
-- DATE: 11.21.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool transmit_packet(char* data) (old)
--            bool transmit_packet(Globals * globals, const char* data)
--
-- RETURNS: bool.
--
-- NOTES: Writes a packet to serial port
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool transmit_packet(Globals * globals, char* data)
{
	//MessageBox(NULL, TEXT("Sending packet"), TEXT(""), MB_OK);
	DWORD bytes = 0;

	data[1] = send_control;
	//send_control = send_control == SOT1 ? SOT2 : SOT1;
	
	WriteFile(*(globals->hComm), data, PACKET_SIZE, &bytes, &ov);
	return wait_for_acknowledgement(globals);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: wait_for_acknowledgement
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.26.13 - added WaitForSingleObject
--					   - handle naks
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool wait_for_acknowledgement() (old)
--            bool wait_for_acknowledgement(Globals *globals)
--
-- RETURNS: bool.
--
-- NOTES: waits for an acknowledgement event from recieve function and returns true if an ACK was recieved
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool wait_for_acknowledgement(Globals *globals)
{
	//MessageBox(NULL, TEXT("WAIT"), TEXT(""), MB_OK);
	DWORD dwWaitResult = WaitForSingleObject(*(globals->hSem), TIMEOUT_TIME);
	// wait for ack (return true of ack)
	switch(dwWaitResult)
	{
	case WAIT_OBJECT_0:  
		//MessageBox(NULL, TEXT("GOT ACK"), TEXT(""), MB_OK);
		//stats.totalNAKsReceived_++;
		//UpdateStats();
		
		// returns true if ack and false if nak
		return globals->gotAck; 
		// nothing received
	case WAIT_ABANDONED: 
		return false;  
	}

	return false;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendControlChar
--
-- DATE: 11.27.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool sendControlChar(HANDLE hComm, char control)
--
-- RETURNS: void.
--
-- NOTES: sends a controll character
-- 
----------------------------------------------------------------------------------------------------------------------*/
void sendControlChar(HANDLE *hComm, char control)
{
	char c[2];
	c[0] = SYN;
	c[1] = control;
	WriteFile(*hComm, c, 2, &dwBytesWritten, &ov);
}