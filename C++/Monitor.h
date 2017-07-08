#pragma once

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>

#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max
int HL_SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
int HL_SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

#else

int HL_SCREEN_WIDTH = 1366;
int HL_SCREEN_HEIGHT = 768;

#endif // defined(WIN32) || defined(_WIN32)
