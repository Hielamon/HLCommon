#pragma once

#include <chrono>
#include <string>
#include <iostream>
#include <cassert>


//#define HL_CONNET(x, y) x##y
//#define HL_CONNET2(x, y) HL_CONNET(x, y)
//#define HL_CONNET_LINE(x) HL_CONNET2(x, __LINE__)

//HL_INTERVAL only can be declared once in main cpp file,
//and this file must be included in main cpp file
//#ifdef MAIN_FILE
//#define HL_INTERVAL_EXTERN
//#else
//#define HL_INTERVAL_EXTERN extern
//#endif // HL_INTERVAL

#define HL_TIME_TEST 1
#define HL_LOG 1

//The class for holding the global variable for time interval computation
//Replacing the following variable
//HL_INTERVAL_EXTERN std::chrono::time_point<std::chrono::steady_clock> hl_interval_start, hl_interval_end;
//HL_INTERVAL_EXTERN std::chrono::nanoseconds hl_interval_cost;
class TimeLog
{
public:
	~TimeLog() {}

	static TimeLog& getInstance()
	{
		static TimeLog tlog;
		return tlog;
	}

	std::chrono::time_point<std::chrono::steady_clock> hl_interval_start, hl_interval_end;
	std::chrono::nanoseconds hl_interval_cost;

private:
	TimeLog() {}
};


#if HL_TIME_TEST

#include <Windows.h>
#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max

//#define HL_INTERVAL_START hl_interval_start = std::chrono::high_resolution_clock::now();
//#define HL_INTERVAL_END \
//hl_interval_end = std::chrono::high_resolution_clock::now(); \
//hl_interval_cost = std::chrono::duration_cast<std::chrono::nanoseconds>(hl_interval_end - hl_interval_start); \
//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_GREEN); \
//std::cout << __FUNCTION__ << "(line : "<< __LINE__<< "£© costed time = "<<hl_interval_cost.count() * 1e-6 << " ms" << std::endl;\
//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);

#define HL_TIME_S TimeLog::getInstance().hl_interval_start
#define HL_TIME_E TimeLog::getInstance().hl_interval_end
#define HL_TIME_C TimeLog::getInstance().hl_interval_cost

#define HL_INTERVAL_START HL_TIME_S = std::chrono::high_resolution_clock::now();

#define HL_INTERVAL_END \
HL_TIME_E = std::chrono::high_resolution_clock::now(); \
HL_TIME_C = std::chrono::duration_cast<std::chrono::nanoseconds>(HL_TIME_E - HL_TIME_S); \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_GREEN); \
std::cout << __FUNCTION__ << "(line : "<< __LINE__<< "£© costed time = "<<HL_TIME_C.count() * 1e-6 << " ms" << std::endl;\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);


#define HL_INTERVAL_ENDSTR(str) \
HL_TIME_E = std::chrono::high_resolution_clock::now(); \
HL_TIME_C = std::chrono::duration_cast<std::chrono::nanoseconds>(HL_TIME_E - HL_TIME_S); \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_GREEN); \
std::cout << __FUNCTION__ << "(line : "<< __LINE__<< "£©" << str <<" costed time = "<<HL_TIME_C.count() * 1e-6 << " ms" << std::endl;\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);


#define IntevalTime(sentence) { clock_t start = clock(); \
sentence; \
clock_t end = clock(); \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_GREEN); \
std::cout <<"\" "<< #sentence <<" \""<< " cost time :"<< end - start <<" ms"<<std::endl; \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE); \
}

#else

#define HL_INTERVAL_START 
#define HL_INTERVAL_END

#define IntevalTime(sentence) sentence

#define HL_INTERVAL_ENDSTR(str)

#endif // HL_TIME_TEST


#if HL_LOG
#define HL_CERR( expr ) {\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_RED);\
std::cerr<< expr << ". " << __FUNCTION__ \
		<< ", " << __FILE__<< ", " << __LINE__ << std::endl; \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);\
exit(-1);}

#define HL_CERR_RETURN_FALSE( expr ) {\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_RED);\
std::cout<< expr << ". " << __FUNCTION__ \
		<< ", " << __FILE__<< ", " << __LINE__ << std::endl; \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);\
return false;}

#define HL_WARNING( expr ) {\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_BLUE);\
std::cout<< expr << ". " << __FUNCTION__ \
		<< ", " << __FILE__<< ", " << __LINE__ << std::endl; \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);\
}

#define HL_GENERAL_LOG( expr ) {\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | */FOREGROUND_GREEN | FOREGROUND_RED);\
std::cout<< expr << std::endl; \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);\
}

#define HL_GENERAL_RED_LOG( expr ) {\
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY | FOREGROUND_GREEN |*/ FOREGROUND_RED);\
std::cout<< expr << std::endl; \
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), /*FOREGROUND_INTENSITY |*/ FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);\
}

#endif // HL_LOG






