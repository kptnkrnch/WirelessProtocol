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

TCHAR Name[] = TEXT("Comm Shell");
char str[80] = "";
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
#pragma warning (disable: 4096)

TCHAR	lpszCommName1[] = TEXT("com1");
TCHAR	lpszCommName2[] = TEXT("com2");
TCHAR	lpszCommName3[] = TEXT("com3");
TCHAR	lpszCommName4[] = TEXT("com4");
int curCom = 0; //The last com port set up
bool comset = false; //Conditional checking if at least one com port has been set
bool KillReader = false; //used for killing the Reading thread
COMMCONFIG	cc; //Com port configuration
HANDLE hComm; //Handle to the com port
HANDLE opThrd; //Handle to the reading thread
DWORD opThrdID;

DWORD WINAPI OutputThread(LPVOID);

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
	OVERLAPPED ov = {0,0,0};
	
	switch (Message)
	{
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
					
            		if (!CommConfigDialog (lpszCommName4, hwnd, &cc))
               			break;
					else {
						comset = true;
						curCom = 4;
					}
				break;
				case IDM_Connect:
					if (comset) {
						if (curCom == 1) {
							if ((hComm = CreateFile (lpszCommName1, GENERIC_READ | GENERIC_WRITE, 0,
   									NULL, OPEN_EXISTING, NULL, NULL))
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
   									NULL, OPEN_EXISTING, NULL, NULL))
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
							opThrd = CreateThread(NULL, 0, OutputThread, (LPVOID)hwnd, 0, &opThrdID );
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
			}
		break;
		case WM_CHAR:	// Process keystroke
			WriteFile(hComm, &wParam, 1, &bytesWritten, NULL);
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: OutputThread
--
-- DATE: September 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: 0
--
-- NOTES:
-- Application's reading thread. This function reads data from the serial port and then outputs it to the screen.
-- This continues until the disconnect option is selected which triggers a conditional that exits the reading
-- loop and then exits the thread.
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI OutputThread(LPVOID n) {
	DWORD bytesRead;
	HWND hwnd = (HWND)n;
	HDC hdc;
	char c = 0;
	int xpos = 1;
	while(!KillReader) { //Reads until the disconnect option is selected.
		ReadFile(hComm, &c, 1, &bytesRead, NULL); //reads from the serial port
		if (c != 0) {
			hdc = GetDC(hwnd); // get device context
			wchar_t chr;
			mbstowcs(&chr, &c, 1);
			TextOut (hdc, xpos, 1, &chr, 1);// output character
			xpos += 11;
			ReleaseDC (hwnd, hdc); // Release device context*/
		}
		c = 0;
		Sleep(200); //frees up some time so the application can write to the serial port
	}
	KillReader = false;
	return 0;
}