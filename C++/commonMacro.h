#pragma once

#include <chrono>
#include <iostream>

//#define HL_CONNET(x, y) x##y
//#define HL_CONNET2(x, y) HL_CONNET(x, y)
//#define HL_CONNET_LINE(x) HL_CONNET2(x, __LINE__)

//HL_INTERVAL only can be declared once in main cpp file,
//and this file must be included in main cpp file
#ifdef MAIN_FILE
#define HL_INTERVAL_EXTERN
#else
#define HL_INTERVAL_EXTERN extern
#endif // HL_INTERVAL

#define HL_TIME_TEST

HL_INTERVAL_EXTERN std::chrono::time_point<std::chrono::steady_clock> hl_interval_start, hl_interval_end;
HL_INTERVAL_EXTERN std::chrono::nanoseconds hl_interval_cost;

#ifdef HL_TIME_TEST

#include <Windows.h>
#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max

#define HL_INTERVAL_START hl_interval_start = std::chrono::high_resolution_clock::now();
#define HL_INTERVAL_END \
hl_interval_end = std::chrono::high_resolution_clock::now(); \
hl_interval_cost = std::chrono::duration_cast<std::chrono::nanoseconds>(hl_interval_end - hl_interval_start); \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_GREEN); \
std::cout << __FUNCTION__ << "(line : "<< __LINE__<< "£© costed time = "<<hl_interval_cost.count() * 1e-6 << " ms" << std::endl;\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);

#else

#define HL_INTERVAL_START 
#define HL_INTERVAL_END

#endif // HL_TIME_TEST




