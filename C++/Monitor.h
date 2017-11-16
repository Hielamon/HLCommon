#pragma once


#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>

#ifdef min
#undef min
#endif // min

#ifdef max
#undef max
#endif // max

#define HL_SCREEN_WIDTH GetSystemMetrics(SM_CXSCREEN)
#define HL_SCREEN_HEIGHT GetSystemMetrics(SM_CYSCREEN)

#else

#define HL_SCREEN_WIDTH 1366
#define HL_SCREEN_HEIGHT 768

#endif // defined(WIN32) || defined(_WIN32)


#include <algorithm>

//Check and Change the inSize to be include in the monitor, and return the changing scale
inline double FitSizeToScreen(int &width, int &height, double spaceRatio = 0.1)
{
	//space emptied : pixel
	int edgeWidth = std::min(HL_SCREEN_WIDTH, HL_SCREEN_HEIGHT) * spaceRatio;
	double scale = 1.0;
	if (width > (HL_SCREEN_WIDTH - edgeWidth))
	{
		scale = (HL_SCREEN_WIDTH - edgeWidth) / double(width);
		width *= scale;
		height *= scale;
	}

	if (height > (HL_SCREEN_HEIGHT - edgeWidth))
	{
		scale = (HL_SCREEN_HEIGHT - edgeWidth) / double(height);
		width *= scale;
		height *= scale;
	}

	return scale;
}
