#define STRICT

#include <windows.h>
#include <stdio.h>
#include "winmenu2.h"
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include "packetize.h"

class Stats {
public:
	Stats():totalPacketsSent_(0),
			totalPacketsReceived_(0),
			totalErrors_(0),
			totalACKs_(0),
			totalNAKs_(0),
			totalTimeouts_(0),
			totalPadding_(0),
			totalRequests_(0),
			elapsedTime_(0),
			usefulBitsSent_(0),
			usefulBitsReceived_(0),
			totalResponseTime_(0) {}

	double SendProtocolEfficiency() const { 
		if (((((totalPacketsSent_ * (1024 - 48)) - totalPadding_)) == 0) || (totalPacketsSent_ * 1024) == 0) {
			return 0;
		} else {
			return ( ((double)((totalPacketsSent_ * (1024 - 48)) - totalPadding_)) / (totalPacketsSent_ * 1024) );
		}
	}
	double ReceiveProtocolEfficiency() const { 
		if (((((totalPacketsReceived_ * (1024 - 48)) - totalPadding_)) == 0) || (totalPacketsReceived_ * 1024) == 0) {
			return 0;
		} else {
			return ( ((double)((totalPacketsReceived_ * (1024 - 48)) - totalPadding_)) / (totalPacketsReceived_ * 1024) ); 
		}
	}
	int EffectiveSendBPS() const { return ((int)(1024 * SendProtocolEfficiency())); }
	int EffectiveReceiveBPS() const { return ((int)(1024 * ReceiveProtocolEfficiency())); }
	int NumPacketsSent() const { return totalPacketsSent_; }
	int BitErrorRate() const { 
		if ((totalPacketsSent_ + totalPacketsReceived_) == 0 || totalErrors_ == 0) {
			return 0;
		} else {
			return (totalErrors_/(totalPacketsSent_ + totalPacketsReceived_));
		}
	}
	int NumACKs() const { return totalACKs_; }
	int NumNAKs() const { return totalNAKs_; }
	int TotalBitsSent() const { return (1024 * totalPacketsSent_); }
	int TotalBitsReceived() const { return (1024 * totalPacketsReceived_); }
	int ResponseTime() const { 
		if (totalResponseTime_ == 0 || totalPacketsReceived_ == 0) {
			return 0;
		} else {
			return totalResponseTime_/totalPacketsReceived_; 
		}
	}
	int ElapsedTransferTime() const { return elapsedTime_; }
	int TotalTimeouts() const { return totalTimeouts_; }
	int TotalRequests() const { return totalRequests_; }
	int AverageSendPadding() const { 
		if (totalPacketsSent_ == 0 || totalPadding_ == 0) {
			return 0;
		} else {
			return (totalPacketsSent_/totalPadding_); 
		}
	}
	int AverageReceivePadding() const { 
		if (totalPacketsReceived_ == 0 || totalPadding_ == 0) {
			return 0;
		} else {
			return (totalPacketsReceived_/totalPadding_); 
		}
	}
	int PacketsSentPerSecond() const { 
		if (totalPacketsSent_ == 0 || elapsedTime_ == 0) {
			return 0;
		} else {
			return (totalPacketsSent_/elapsedTime_); 
		}
	}
	int PacketsReceivedPerSecond() const { 
		if (totalPacketsReceived_ == 0 || elapsedTime_ == 0) {
			return 0;
		} else {
			return (totalPacketsReceived_/elapsedTime_); 
		}
	}
private:
	int totalPacketsSent_;
	int totalPacketsReceived_;
	int totalErrors_;
	int totalACKs_;
	int totalNAKs_;
	int totalTimeouts_;
	int totalPadding_;
	int totalRequests_;
	int elapsedTime_;
	int usefulBitsSent_;
	int usefulBitsReceived_;
	int totalResponseTime_;
};

TCHAR Name[] = TEXT("Be Creative");
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
extern HANDLE hComm; //Handle to the com port
HANDLE opThrd; //Handle to the reading thread
HANDLE PollingThrd; //Handle to the reading thread
DWORD opThrdID;
DWORD PollingThrdID;
bool areaset = false;
int iVertPos = 0, iHorzPos = 0;
RECT text_area;

DWORD WINAPI OutputThread(LPVOID);
DWORD WINAPI PollingThread(LPVOID);
std::fstream OpenFile(std::string FileLocation);
HANDLE& OpenConnection(HANDLE& comm);
void DisplayStatistics(const RECT& rect, const Stats& stats, HDC& hdc);
void DisplayReceivedFileData(const RECT& rect, std::vector<std::string> FileContents, HDC& hdc);

void DisplayElapsedTime(int time, HDC& hdc, int x, int& y);
void DisplaySendProtocolEfficiency(double eff, HDC& hdc, int x, int& y);
void DisplayReceiveProtocolEfficiency(double eff, HDC& hdc, int x, int& y);
void DisplayEffectiveSendBPS(int bps, HDC& hdc, int x, int& y);
void DisplayEffectiveReceiveBPS(int bps, HDC& hdc, int x, int& y);
void DisplayNumPacketsSent(int count, HDC& hdc, int x, int& y);
void DisplayBitErrorRate(int count, HDC& hdc, int x, int& y);
void DisplayNumACKs(int count, HDC& hdc, int x, int& y);
void DisplayNumNAKs(int count, HDC& hdc, int x, int& y);
void DisplayTotalBitsSent(int count, HDC& hdc, int x, int& y);
void DisplayTotalBitsReceived(int count, HDC& hdc, int x, int& y);
void DisplayResponseTime(int time, HDC& hdc, int x, int& y);
void DisplayTotalTimeouts(int count, HDC& hdc, int x, int& y);
void DisplayTotalRequests(int count, HDC& hdc, int x, int& y);
void DisplayAverageSendPadding(int count, HDC& hdc, int x, int& y);
void DisplayAverageReceivePadding(int count, HDC& hdc, int x, int& y);
void DisplayPacketsSentPerSecond(int pps, HDC& hdc, int x, int& y);
void DisplayPacketsReceivedPerSecond(int pps, HDC& hdc, int x, int& y);
//bool SendFile(std::ifstream* file);
bool isStop = false;

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

LRESULT CALLBACK WndProc (HWND hwnd, UINT Message,
                          WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static int  cxClient,cyClient;
	SCROLLINFO si;
	LPDCB lpDCB = &cc.dcb;
	char buffer[128] = {0};
	DWORD bytesRead;
	DWORD bytesWritten;
	OVERLAPPED ov = {0,0,0};
	PAINTSTRUCT ps;
	RECT topleft;
	RECT topright;
	RECT bottomleft;
	RECT bottomright;
	Stats stats;
	HANDLE _font;
	BOOL fRedraw;
	std::vector<std::string> file;
	file.push_back("Hello world, this is a line of text for testing purposes, it is gunna be fucking huge. Winter is coming. Weiner Weiner Weiner.");
	std::fstream ifs;
	OPENFILENAME ofn;
	static TCHAR szFilter[] = TEXT ("All Files (*.*)\0*.*\0\0") ;
	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH] ;


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
					
            		if (!CommConfigDialog (lpszCommName4, hwnd, &cc))
               			break;
					else {
						comset = true;
						curCom = 4;
					}
				break;
				case IDM_Connect:

					OpenConnection(hComm);

				break;
				case IDM_Disconnect:
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

						for(auto x: wfilename) {
							filename += x;
						}
						
						ifs = OpenFile(filename);


						readFile(ifs);
					}

				break;
			}
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
		case WM_CHAR:
		break;
		
		case WM_DESTROY:	// Terminate program
      		PostQuitMessage (0);
		break;

		case WM_PAINT:
			hdc = BeginPaint(hwnd, &ps);
			MoveToEx(hdc, cxClient - cxClient/3, 0, NULL);
			LineTo(hdc, cxClient - cxClient/3, cyClient);
			MoveToEx(hdc, 0, cyClient - cyClient/3, NULL);
			LineTo(hdc, cxClient, cyClient - cyClient/3);
			DisplayStatistics(topright, stats, hdc);
			DisplayReceivedFileData(topleft, file, hdc);
			ReleaseDC(hwnd, hdc);
		break;
		default:
			return DefWindowProc (hwnd, Message, wParam, lParam);
	}
	return 0;
}

std::fstream OpenFile(std::string FileLocation) {
	std::fstream file(FileLocation);
	return file;
}

HANDLE& OpenConnection(HANDLE& comm) {
	if ((hComm = CreateFile (lpszCommName4, GENERIC_READ | GENERIC_WRITE, 0,
   			NULL, OPEN_EXISTING, NULL, NULL))
            == INVALID_HANDLE_VALUE)
	{
   		MessageBox (NULL, TEXT("Error opening COM port:"), TEXT(""), MB_OK);
		throw "Error opening COM port!";
	}
}

void CloseConnection(HANDLE& comm) {
	MessageBox (NULL, TEXT("disconnected"), TEXT(""), MB_OK);
	if (comm != NULL) {
		CloseHandle(comm);
	}
}

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
	DisplayNumACKs(stats.NumACKs(), hdc, xpos, ypos);
	DisplayNumNAKs(stats.NumNAKs(), hdc, xpos, ypos);
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





