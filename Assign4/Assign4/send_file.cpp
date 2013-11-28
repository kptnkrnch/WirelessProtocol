/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: 
--
-- PROGRAM: 
--
-- FUNCTIONS:
-- 
--
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
#include <iostream>
#include <windows.h>

#define SYN 0x16
#define ENQ 0x05
#define EOT 0x04

using namespace std;

DWORD dwBytesWritten;
DWORD dwWaitResult; 

extern HANDLE hSem;
extern HANDLE hComm;

void send_file(char* data[]);
bool enquire_line();
bool transmit_packet(char* data);
bool wait_for_acknowledgement();

void sendEOT(HANDLE hComm);
bool sendENQ(HANDLE hComm);

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: send_file
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.23.13 - replaced leftover pseudocode
--            11.27.13 - added semaphores and error checking
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: void send_file(char* data[]);
--
-- RETURNS: void.
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
void send_file(char* data[])
{
    int i = 0;
    int packets_sent = 0;

    if(!enquire_line())
        cerr << "could not enquire line" << endl;
    // loop through all the data
    while(*data[i] != '\n')
    {
        if((packets_sent < 5))
        {
        	if(!transmit_packet(data[i]))
                cerr << "error sending packet" << i << endl;
            i++;
        }else
        {
            // send end of transmition
            //WriteFile(hCommPort, EOT, 2, &dwBytesWritten, NULL);
			sendEOT(hComm);
            ReleaseSemaphore(hSem, 1, NULL);
            if(!wait_for_acknowledgement())
                cerr << "timeout" << endl;
            packets_sent = 0;
        }
        packets_sent++;
    }
    // send end of transmition
    //WriteFile(hComm, EOT, 2, &dwBytesWritten, NULL);
	sendEOT(hComm);
    ReleaseSemaphore(hSem, 1, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: enquire_line
--
-- DATE: 11.21.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool enquire_line();
--
-- RETURNS: bool.
--
-- NOTES: 
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool enquire_line()
{
    // send enq
    //if(!WriteFile(hComm, ENQ, 2, &dwBytesWritten, NULL))
    //    return false;
	
	if (!sendENQ(hComm))
		return false;

    // wait for reciever to acknowledge line is free
    return wait_for_acknowledgement();;
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
-- INTERFACE: bool transmit_packet(char* data);
--
-- RETURNS: bool.
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool transmit_packet(char* data)
{
    // write data to port
	return WriteFile(hComm, &data,(DWORD) sizeof(data), &dwBytesWritten, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: wait_for_acknowledgement
--
-- DATE: 11.21.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool wait_for_acknowledgement();
--
-- RETURNS: bool.
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool wait_for_acknowledgement()
{

    dwWaitResult = WaitForSingleObject(hSem,INFINITE);

    switch(dwWaitResult)
    {
        case WAIT_OBJECT_0:            
            return true; 

        case WAIT_ABANDONED: 
            return false;  
    }
}

void sendEOT(HANDLE hComm){
	char c[2];
	c[0] = SYN;
	c[1] = EOT;
	DWORD bytesRead;
	WriteFile(hComm, c, 2, &bytesRead, NULL);
}

bool sendENQ(HANDLE hComm){
	char c[2];
	c[0] = SYN;
	c[1] = ENQ;
	DWORD bytesRead;
	return WriteFile(hComm, c, 2, &bytesRead, NULL);
}