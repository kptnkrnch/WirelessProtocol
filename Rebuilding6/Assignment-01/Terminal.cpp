/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Terminal.cpp - An application that transfers a file's contents using our class designed protocol.
--
-- PROGRAM: Be Creative
--
-- FUNCTIONS:
-- int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
-- LRESULT CALLBACK WndProc (HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
-- void DisplayStatistics(const RECT& rect, const Stats& stats, HDC& hdc)
-- void DisplayReceivedFileData(const RECT& rect, std::vector<std::string> FileContents, HDC& hdc)
-- void DisplayElapsedTime(int time, HDC& hdc, int x, int& y)
-- void DisplaySendProtocolEfficiency(double eff, HDC& hdc, int x, int& y)
-- void DisplayReceiveProtocolEfficiency(double eff, HDC& hdc, int x, int& y)
-- void DisplayEffectiveSendBPS(int bps, HDC& hdc, int x, int& y)
-- void DisplayEffectiveReceiveBPS(int bps, HDC& hdc, int x, int& y)
-- void DisplayNumPacketsSent(int count, HDC& hdc, int x, int& y)
-- void DisplayBitErrorRate(int count, HDC& hdc, int x, int& y)
-- void DisplayNumACKs(int count, HDC& hdc, int x, int& y)
-- void DisplayNumNAKs(int count, HDC& hdc, int x, int& y)
-- void DisplayTotalBitsSent(int count, HDC& hdc, int x, int& y)
-- void DisplayTotalBitsReceived(int count, HDC& hdc, int x, int& y)
-- void DisplayResponseTime(int time, HDC& hdc, int x, int& y)
-- void DisplayTotalTimeouts(int count, HDC& hdc, int x, int& y)
-- void DisplayTotalRequests(int count, HDC& hdc, int x, int& y)
-- void DisplayAverageSendPadding(int count, HDC& hdc, int x, int& y)
-- void DisplayAverageReceivePadding(int count, HDC& hdc, int x, int& y)
-- void DisplayPacketsSentPerSecond(int pps, HDC& hdc, int x, int& y)
-- void DisplayPacketsReceivedPerSecond(int pps, HDC& hdc, int x, int& y)
-- void UpdateStats()
-- void UpdateFile()
--
--
-- DATE: November 29, 2013
--
-- REVISIONS: None
--
-- DESIGNERS: Joshua Campbell, Jordan Marling, Damien Sathanielle, Cory Thomas
--
-- PROGRAMMERS: Joshua Campbell, Jordan Marling, Damien Sathanielle, Cory Thomas
--
-- NOTES:
-- Program works successfully over wireless or serial. Sending files at the same time causes an error.
----------------------------------------------------------------------------------------------------------------------*/

#define STRICT

#include <windows.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

#include "global.h"
#include "sendFile.h"
#include "packetize.h"
#include "Receiver.h"
#include "Terminal.h"
#include "winmenu2.h"


TCHAR Name[] = TEXT("Be Creative");
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

Stats stats;

HWND hwnd;

RECT topleft;
RECT topright;
RECT bottomleft;
RECT bottomright;

std::vector<std::string> lines;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: OpenFile
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Jordan Marling
--
-- PROGRAMMER: Jordan Marling
--
-- RETURNS: an fstream object for the file
--
-- NOTES:
-- Used for opening a file using a location path
----------------------------------------------------------------------------------------------------------------------*/
std::fstream OpenFile(std::string FileLocation) {
	std::fstream file(FileLocation);
	return file;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: November 29, 2013
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
		1024, 800, NULL, NULL, hInst, NULL);
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
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell, Jordan Marling
--
-- RETURNS: False or 0
--
-- NOTES:
-- The main procedure for the application. Handles setting up COM ports, connecting, initializing semaphores,
-- initializing threads, opening files, etc.
----------------------------------------------------------------------------------------------------------------------*/

LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	LPDCB lpDCB = &cc.dcb;
	static int  cxClient,cyClient;
	PAINTSTRUCT ps;
	static int iVertPos = 0;
	SCROLLINFO si;
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

	if (cxClient && cyClient) {
		/*text_area.left = 0;
		text_area.top = 0;
		text_area.right = cxClient;
		text_area.bottom = cyClient;
		areaset = true;*/
		topright.left = cxClient - cxClient/3 + 1;
		topright.top = 1;
		topright.right = cxClient;
		topright.bottom = cyClient - cyClient/3 - 1;
		topleft.left = 1;
		topleft.top = 1;
		topleft.right = cxClient - cxClient/3 - 1;
		topleft.bottom = cyClient - cyClient/3 - 1;
	}

	switch (Message)
	{
	case WM_CREATE:
		ov.Offset = 0;
		ov.OffsetHigh = 0;
		ov.Pointer = 0;
		ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		global.hSem = (HANDLE*)malloc(sizeof(HANDLE));
		*global.hSem = CreateSemaphore(NULL, 0, 1, NULL);

		//global.hSem = &semm;

		global.ofs.open("file.txt");
		global.ofs.close();
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
					//global.ov = ov;
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
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		si.nMin = 0;
		si.nMax = cyClient;
		si.nPos = 0;
		si.nPage = 50;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		break;
	case WM_VSCROLL:

		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;

		switch(LOWORD(wParam)){

		case SB_LINEUP:
			si.nPos -= 10;
			break;

		case SB_LINEDOWN:
			si.nPos += 10;
			break;

		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;

		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;

		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);

		//if there was change in the vertical scroll bar, make adjustments to redraw
		if (si.nPos != iVertPos){
			//InvalidateRect(hwnd, &text_area, TRUE);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		MoveToEx(hdc, cxClient - cxClient/3, 0, NULL);
		LineTo(hdc, cxClient - cxClient/3, cyClient);
		MoveToEx(hdc, 0, cyClient - cyClient/3, NULL);
		LineTo(hdc, cxClient, cyClient - cyClient/3);
		DisplayStatistics(topright, stats, hdc);
		DisplayReceivedFileData(topleft, lines, hdc);
		ReleaseDC(hwnd, hdc);
		break;
	case WM_DESTROY:	// Terminate program
		if (hComm) {
			//CloseHandle(hComm);
		}
		PostQuitMessage (0);
		break;
	default:
		return DefWindowProc (hwnd, Message, wParam, lParam);
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: UpdateStats
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Jordan Marling
--
-- PROGRAMMER: Jordan Marling
--
-- RETURNS: void
--
-- NOTES:
-- Refreshes the top right corner of the window.
----------------------------------------------------------------------------------------------------------------------*/
void UpdateStats() {

	InvalidateRect(hwnd, &topright, TRUE);

}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: UpdateFile
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Jordan Marling
--
-- PROGRAMMER: Jordan Marling
--
-- RETURNS: void
--
-- NOTES:
-- Refreshes the top left corner of the file.
----------------------------------------------------------------------------------------------------------------------*/
void UpdateFile() {
	InvalidateRect(hwnd, &topleft, TRUE);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayStatistics
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays all of the statistics in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayStatistics(const RECT& rect, const Stats& stats, HDC& hdc) {
	int xpos = rect.left + 1;
	int ypos = rect.top + 1;
	DisplayElapsedTime(stats.ElapsedTransferTime(), hdc, xpos, ypos);
	DisplaySendProtocolEfficiency(stats.SendProtocolEfficiency(), hdc, xpos, ypos);
	DisplayReceiveProtocolEfficiency(stats.ReceiveProtocolEfficiency(), hdc, xpos, ypos);
	DisplayEffectiveSendBPS(stats.EffectiveSendBPS(), hdc, xpos, ypos);
	DisplayEffectiveReceiveBPS(stats.EffectiveReceiveBPS(), hdc, xpos, ypos);
	DisplayNumPacketsSent(stats.NumPacketsSent(), hdc, xpos, ypos);
	DisplayBitErrorRate(stats.BitErrorRate(), hdc, xpos, ypos);
	//DisplayNumACKs(stats.NumACKs(), hdc, xpos, ypos);
	//DisplayNumNAKs(stats.NumNAKs(), hdc, xpos, ypos);
	DisplayTotalBitsSent(stats.TotalBitsSent(), hdc, xpos, ypos);
	DisplayTotalBitsReceived(stats.TotalBitsReceived(), hdc, xpos, ypos);
	DisplayResponseTime(stats.ResponseTime(), hdc, xpos, ypos);
	DisplayTotalTimeouts(stats.TotalTimeouts(), hdc, xpos, ypos);
	DisplayTotalRequests(stats.TotalRequests(), hdc, xpos, ypos);
	DisplayAverageSendPadding(stats.AverageSendPadding(), hdc, xpos, ypos);
	DisplayAverageReceivePadding(stats.AverageReceivePadding(), hdc, xpos, ypos);
	DisplayPacketsSentPerSecond(stats.PacketsSentPerSecond(), hdc, xpos, ypos);
	DisplayPacketsReceivedPerSecond(stats.PacketsReceivedPerSecond(), hdc, xpos, ypos);
}

/*------------------------------START STATS SECTION------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayElapsedTime
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the elapsed time.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayElapsedTime(int time, HDC& hdc, int x, int& y) {
	std::string tmp = "ElapsedTime: ";
	std::ostringstream oss;
	oss << tmp << time;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplaySendProtocolEfficiency
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the send protocol efficiency in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplaySendProtocolEfficiency(double eff, HDC& hdc, int x, int& y) {
	std::string tmp = "SendProtocolEfficiency: ";
	std::ostringstream oss;
	oss << tmp << eff;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayReceiveProtocolEfficiency
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the receive protocol efficiency in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayReceiveProtocolEfficiency(double eff, HDC& hdc, int x, int& y) {
	std::string tmp = "ReceiveProtocolEfficiency: ";
	std::ostringstream oss;
	oss << tmp << eff;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayEffectiveSendBPS
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the effective bps (sending) in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayEffectiveSendBPS(int bps, HDC& hdc, int x, int& y) {
	std::string tmp = "EffectiveSendBPS: ";
	std::ostringstream oss;
	oss << tmp << bps;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayEffectiveReceiveBPS
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the effective bps (receiving) in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayEffectiveReceiveBPS(int bps, HDC& hdc, int x, int& y) {
	std::string tmp = "EffectiveReceiveBPS: ";
	std::ostringstream oss;
	oss << tmp << bps;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayNumPacketsSent
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the number of packets sent in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayNumPacketsSent(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "NumberOfPacketsSent: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayBitErrorRate
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the bit error rate in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayBitErrorRate(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "BitErrorRate: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayNumACKs
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the number of ACKs in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayNumACKs(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "NumberOfACKs: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayNumNAKs
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the number of NAKs in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayNumNAKs(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "NumberOfNAKs: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayTotalBitsSent
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the total bits sent in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayTotalBitsSent(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "TotalBitsSent: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayTotalBitsReceived
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the total bits received in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayTotalBitsReceived(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "TotalBitsReceived: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayResponseTime
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the response time in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayResponseTime(int time, HDC& hdc, int x, int& y) {
	std::string tmp = "ResponseTime: ";
	std::ostringstream oss;
	oss << tmp << time;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayTotalTimeouts
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the total timeouts in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayTotalTimeouts(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "TotalTimeouts: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayTotalRequests
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the total requests in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayTotalRequests(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "TotalRequests: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayAverageSendPadding
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the average sending padding in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayAverageSendPadding(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "AvgSendPadding: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayAverageReceivePadding
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the average receiving padding in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayAverageReceivePadding(int count, HDC& hdc, int x, int& y) {
	std::string tmp = "AvgReceivePadding: ";
	std::ostringstream oss;
	oss << tmp << count;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayPacketsSentPerSecond
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the packets sent per second in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayPacketsSentPerSecond(int pps, HDC& hdc, int x, int& y) {
	std::string tmp = "PacketsSentPerSec: ";
	std::ostringstream oss;
	oss << tmp << pps;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayPacketsReceivedPerSecond
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the packets received per second in the top right corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayPacketsReceivedPerSecond(int pps, HDC& hdc, int x, int& y) {
	std::string tmp = "PacketsReceivedPerSec: ";
	std::ostringstream oss;
	oss << tmp << pps;
	tmp = oss.str();
	std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
	const wchar_t *s = widestr.c_str();
	TextOut(hdc, x, y, (TCHAR*)s, tmp.length());
	y += 22;
}
/*------------------------------------------END STATS SECTION------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DisplayReceivedFileData
--
-- DATE: November 29, 2013
--
-- REVISIONS: none
--
-- DESIGNER: Joshua Campbell
--
-- PROGRAMMER: Joshua Campbell
--
-- RETURNS: void
--
-- NOTES:
-- Displays the received file contents in the top left corner.
----------------------------------------------------------------------------------------------------------------------*/
void DisplayReceivedFileData(const RECT& rect, std::vector<std::string> FileContents, HDC& hdc) {
	int xpos = rect.left + 1;
	int ypos = rect.top + 1;
	int spacing = 9;
	for (std::vector<std::string>::iterator it = FileContents.begin(); it != FileContents.end(); ++it) {
		std::string tmp;
		std::istringstream iss(*it);
		while (iss >> tmp) {
			tmp += " ";
			if ((xpos + (tmp.length() * spacing)) < rect.right) {
				std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
				const wchar_t *s = widestr.c_str();
				TextOut(hdc, xpos, ypos, (TCHAR*)s, tmp.length());
				xpos += (tmp.length() * spacing);
			} else {
				ypos += 18;
				xpos = rect.left + 1;
				std::wstring widestr = std::wstring(tmp.begin(), tmp.end());
				const wchar_t *s = widestr.c_str();
				TextOut(hdc, xpos, ypos, (TCHAR*)s, tmp.length());
				xpos += (tmp.length() * spacing);
			}
			tmp.clear();
		}
	}
}


void DisplayFileProgressStats(const RECT& rect) {
}

void DisplayProtocolOperations(const RECT& rect) {
}