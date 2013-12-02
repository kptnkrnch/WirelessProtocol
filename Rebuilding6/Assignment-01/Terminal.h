#ifndef TERMINAL_H
#define TERMINAL_H

#include <Windows.h>
#include <vector>

#include "global.h"

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

void UpdateStats();
void UpdateFile();

#endif