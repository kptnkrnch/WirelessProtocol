/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Terminal.cpp - An application that establishes a connection via a computer's
-- serial port and then sends and receives text input.
--
-- PROGRAM: Dumb Terminal
--
-- FUNCTIONS:
-- int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- LRESULT CALLBACK WndProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
-- DWORD WINAPI OutputThread(LPVOID n)
--
--
-- DATE: September 29, 2013
--
-- REVISIONS: None
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- NOTES:
-- The program will require the user to correctly configure the port settings.
-- In order to initiate a connection, a user will have to configure the port and then select the "connect"
-- option from the "Options" menu.
-- You cannot open more than one port at a time. You will have to select the "disconnect" first before
-- opening a different port.
----------------------------------------------------------------------------------------------------------------------*/

#define STRICT

#include <windows.h>
#include <stdio.h>
#include "winmenu2.h"
#include <string>
#include "global.h"
#include <fstream>
#include <vector>
#include <sstream>

#include "sendFile.h"
#include "packetize.h"
#include "Receiver.h"


TCHAR Name[] = TEXT("Comm Shell");
char str[80] = "";
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
#pragma warning (disable: 4096)

TCHAR	lpszCommName1[] = TEXT("com1");
TCHAR	lpszCommName2[] = TEXT("com2");
TCHAR	lpszCommName3[] = TEXT("com3");
TCHAR	lpszCommName4[] = TEXT("com4");
HANDLE sendThread = 0;
HANDLE recvThread = 0;
int curCom = 0; //The last com port set up
bool comset = false; //Conditional checking if at least one com port has been set
bool KillReader = false; //used for killing the Reading thread
COMMCONFIG	cc; //Com port configuration
HANDLE hComm; //Handle to the com port
HANDLE opThrd; //Handle to the reading thread
DWORD opThrdID;
OVERLAPPED ov;
Globals global;
HANDLE semm;

//DWORD WINAPI OutputThread(LPVOID);
std::fstream OpenFile(std::string FileLocation) {
	std::fstream file(FileLocation);
	return file;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: September 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: A Windows message wParam.
--
-- NOTES:
-- The Main function for the program. Responsible for creating the Windows window for the application.
----------------------------------------------------------------------------------------------------------------------*/

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance,
 						  LPSTR lspszCmdParam, int nCmdShow)
{
	HWND hwnd;
	MSG Msg;
	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof (WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style
	
	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;
	
	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0; 
	
	if (!RegisterClassEx (&Wcl))
		return 0;

	hwnd = CreateWindow (Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
   							600, 400, NULL, NULL, hInst, NULL);
	ShowWindow (hwnd, nCmdShow);
	UpdateWindow (hwnd);

	while (GetMessage (&Msg, NULL, 0, 0))
	{
   		TranslateMessage (&Msg);
		DispatchMessage (&Msg);
	}

	return Msg.wParam;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: September 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: False or 0
--
-- NOTES:
-- The main procedure for the application. Handles setting up COM ports, connecting, disconnecting, writing to the
-- serial port, and closing threads and connections when the program ends.
----------------------------------------------------------------------------------------------------------------------*/

LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
                          WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	LPDCB lpDCB = &cc.dcb;
	char buffer[128] = {0};
	DWORD bytesRead;
	DWORD bytesWritten;
	//OVERLAPPED ov = {0,0,0};
	std::fstream ifs;
	OPENFILENAME ofn;
	static TCHAR szFilter[] = TEXT ("All Files (*.*)\0*.*\0\0") ;
	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH] ;
	char packet[1024];
	char packet2[2];
	
	switch (Message)
	{
		case WM_CREATE:
			ov.Offset = 0;
			ov.OffsetHigh = 0;
			ov.Pointer = 0;
			ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			semm = CreateSemaphore(NULL, 0, 1, NULL);
			global.hSem = &semm;
		break;
		case WM_COMMAND:
			switch (LOWORD (wParam))
			{
          		case IDM_COM1:
            		cc.dwSize = sizeof(COMMCONFIG);
					cc.wVersion = 0x100;

					if (!CommConfigDialog (lpszCommName1, hwnd, &cc))
               			break;
					else {
						comset = true;
						curCom = 1;
					}
				break;
				case IDM_COM2:
					cc.dwSize = sizeof(COMMCONFIG);
					cc.wVersion = 0x100;
					
            		if (!CommConfigDialog (lpszCommName2, hwnd, &cc))
               			break;
					else {
						comset = true;
						curCom = 2;
					}
				break;
				case IDM_COM3:
					cc.dwSize = sizeof(COMMCONFIG);
					cc.wVersion = 0x100;
					
            		if (!CommConfigDialog (lpszCommName3, hwnd, &cc))
               			break;
					else {
						comset = true;
						curCom = 3;
					}
				break;
				case IDM_COM4:
					cc.dwSize = sizeof(COMMCONFIG);
					cc.wVersion = 0x100;
					
            		if (!CommConfigDialog (lpszCommName4, hwnd, &cc)) {
               			break;
					}else {
						comset = true;
						curCom = 4;
					}
				break;
				case IDM_Connect:
					//WaitForSingleObject(ov.hEvent, INFINITE);
					//MessageBox(hwnd, TEXT(""), TEXT(""), MB_OK);
					if (comset) {
						if (curCom == 1) {
							if ((hComm = CreateFile (lpszCommName1, GENERIC_READ | GENERIC_WRITE, 0,
   									NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
                        			== INVALID_HANDLE_VALUE)
							{
   								MessageBox (NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
								return FALSE;
							}
						} else if (curCom == 2) {
							if ((hComm = CreateFile (lpszCommName2, GENERIC_READ | GENERIC_WRITE, 0,
   									NULL, OPEN_EXISTING, NULL, NULL))
                        			== INVALID_HANDLE_VALUE)
							{
   								MessageBox (NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
								return FALSE;
							}
						} else if (curCom == 3) {
							if ((hComm = CreateFile (lpszCommName3, GENERIC_READ | GENERIC_WRITE, 0,
   									NULL, OPEN_EXISTING, NULL, NULL))
                        			== INVALID_HANDLE_VALUE)
							{
   								MessageBox (NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
								return FALSE;
							}
						} else if (curCom == 4) {
							if ((hComm = CreateFile (lpszCommName4, GENERIC_READ | GENERIC_WRITE, 0,
   									NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL))
                        			== INVALID_HANDLE_VALUE)
							{
   								MessageBox (NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
								return FALSE;
							}
						}
						lpDCB = &cc.dcb;

						if (!SetCommState(hComm, lpDCB)) {
							MessageBox (NULL, TEXT("Error setting COM state"), TEXT(""), MB_OK);
							return false;
						}

						COMMTIMEOUTS timeouts;
		
						timeouts.ReadIntervalTimeout = MAXDWORD; 
						timeouts.ReadTotalTimeoutMultiplier	= 0;
						timeouts.ReadTotalTimeoutConstant = 0;
						timeouts.WriteTotalTimeoutMultiplier = 0;
						timeouts.WriteTotalTimeoutConstant = 0;

						if (!SetCommTimeouts(hComm, &timeouts)) {
							MessageBox (NULL, TEXT("Error setting COM timeouts"), TEXT(""), MB_OK);
							return false;
						}
						if (hComm) {
							global.hComm = &hComm;
							global.ov = ov;
							//opThrd = CreateThread(NULL, 0, OutputThread, (LPVOID)hwnd, 0, &opThrdID );
							recvThread = CreateThread(NULL, 0, receiverThread, &global, NULL, &opThrdID);
							MessageBox(NULL, TEXT("connected"), TEXT(""), MB_OK);
						} else {
							MessageBox(NULL, TEXT("Please select a COM port."), TEXT(""), MB_OK);
						}
					} else {
						MessageBox(NULL, TEXT("Please select a COM port."), TEXT(""), MB_OK);
					}
					
				break;
				case IDM_Disconnect: //kills the reading thread and closes the com port if they are active
					if (KillReader == false && hComm) {
						KillReader = true;
						CloseHandle(hComm);
						MessageBox(NULL, TEXT("disconnected"), TEXT(""), MB_OK);
					}
				break;
				case IDM_OpenFile:

					ofn.lStructSize       = sizeof (OPENFILENAME) ;
					ofn.hwndOwner         = hwnd ;
					ofn.hInstance         = NULL ;
					ofn.lpstrFilter       = szFilter ;
					ofn.lpstrCustomFilter = NULL ;
					ofn.nMaxCustFilter    = 0 ;
					ofn.nFilterIndex      = 0 ;
					ofn.lpstrFile         = NULL ;          // Set in Open and Close functions
					ofn.nMaxFile          = MAX_PATH ;
					ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
					ofn.nMaxFileTitle     = MAX_PATH ;
					ofn.lpstrInitialDir   = NULL ;
					ofn.lpstrTitle        = NULL ;
					ofn.Flags             = 0 ;             // Set in Open and Close functions
					ofn.nFileOffset       = 0 ;
					ofn.nFileExtension    = 0 ;
					ofn.lpstrDefExt       = TEXT ("txt") ;
					ofn.lCustData         = 0L ;
					ofn.lpfnHook          = NULL ;
					ofn.lpTemplateName    = NULL ;

					ofn.hwndOwner         = hwnd ;
					ofn.lpstrFile         = szFileName ;
					ofn.lpstrFileTitle    = szTitleName ;
					ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;

					if (GetOpenFileName(&ofn)) {
						
						std::wstring wfilename(szFileName);
						std::string filename;

						for(int i = 0; i < wfilename.size(); i++) {
							filename += wfilename[i];
						}
						
						ifs = OpenFile(filename);

						readFile(ifs);

						DWORD threadID;
						DWORD exitStatus;

						if (sendThread == 0 || (GetExitCodeThread(sendThread, &exitStatus) && exitStatus != STILL_ACTIVE)) {
							sendThread = CreateThread(NULL, 0, sendBufferThread, &global, NULL, &threadID);
						}

					}
				}
				break;
		case WM_CHAR:	// Process keystroke
		break;
		case WM_LBUTTONDOWN:
		break;
		
		case WM_DESTROY:	// Terminate program
			if (hComm) {
				CloseHandle(hComm);
			}
      		PostQuitMessage (0);
		break;
		default:
			return DefWindowProc (hwnd, Message, wParam, lParam);
	}
	return 0;
}