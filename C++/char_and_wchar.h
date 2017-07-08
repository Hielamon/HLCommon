#ifndef CHAR_AND_WCHAR_HEAD
#define CHAR_AND_WCHAR_HEAD


#include <cstdlib>
#include <string>

inline wchar_t * ctow(const char* c, wchar_t * w)
{
	setlocale(LC_ALL, "zh-CN");
	size_t len = strlen(c) + 1;
	size_t converted = 0;
	if (w != NULL)free(w);
	w = (wchar_t *)malloc(len * sizeof(wchar_t));
	mbstowcs_s(&converted, w, len, c, _TRUNCATE);
	setlocale(LC_ALL, "C");
	return w;
}

inline char * wtoc(const wchar_t* w, char * c)
{
	setlocale(LC_ALL, "zh-CN");
	//size_t len = wcslen(w) + 1;
	size_t wsize = wcslen((const wchar_t*)w);
	size_t asize = wsize * 2 + 1;
	size_t converted = 0;
	if (c != NULL)free(c);
	c = (char *)malloc(asize * sizeof(char));
	wcstombs_s(&converted, c, asize, w, asize);
	setlocale(LC_ALL, "C");
	return c;
}

#endif // !CHAR_AND_WCHAR_HEAD
