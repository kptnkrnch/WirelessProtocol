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
#include "sendFile.h"

using namespace std;

DWORD dwBytesWritten;
DWORD dwWaitResult; 

extern HANDLE hSem;
extern HANDLE hComm;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: send_file
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.23.13 - replaced leftover pseudocode
--            11.26.13 - added semaphores and error checking
--            11.27.13 - added buffer and fix ENQ/EOT sending
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
void send_file()
{
    int packets_sent = 0;

    if(!enquire_line())
        cerr << "could not enquire line" << endl;
    // loop through all the data
    while(!buffer.is_empty())
    {
        if((packets_sent < 5))
        {
            if(!transmit_packet(buffer.get_packet()))
                cerr << "error sending packet" << endl;
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
        buffer.remove_packet();
    }
    // send end of transmition
    sendEOT(hComm);
    ReleaseSemaphore(hSem, 1, NULL);
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
bool transmit_packet(const char* data)
{
    // write data to port
    return WriteFile(hComm, &data,(DWORD) sizeof(data), &dwBytesWritten, NULL);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: wait_for_acknowledgement
--
-- DATE: 11.21.13
--
-- REVISIONS: 11.26.13 - added WaitForSingleObject
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
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendEOT
--
-- DATE: 11.27.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: void sendEOT(HANDLE hComm)
--
-- RETURNS: void.
--
-- NOTES: sentds end of transmition character
-- 
----------------------------------------------------------------------------------------------------------------------*/
void sendEOT(HANDLE hComm)
{
    char c[2];
    c[0] = SYN;
    c[1] = EOT;
    DWORD bytesRead;
    WriteFile(hComm, c, 2, &bytesRead, NULL);
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendENQ
--
-- DATE: 11.27.13
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Damien Sathanielle
--
-- PROGRAMMER: Damien Sathanielle
--
-- INTERFACE: bool sendENQ(HANDLE hComm)
--
-- RETURNS: bool.
--
-- NOTES: sends and ENQ character to enquire the line
-- 
----------------------------------------------------------------------------------------------------------------------*/
bool sendENQ(HANDLE hComm)
{
    char c[2];
    c[0] = SYN;
    c[1] = ENQ;
    DWORD bytesRead;
    return WriteFile(hComm, c, 2, &bytesRead, NULL);
}