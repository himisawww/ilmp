#include"CalcTime.h"

#include<windows.h>
double CalcTime(){
	static LARGE_INTEGER time_li;
	static double time_w=(QueryPerformanceFrequency(&time_li),(double)time_li.QuadPart);
	return (QueryPerformanceCounter(&time_li),(double)time_li.QuadPart/time_w);
}
