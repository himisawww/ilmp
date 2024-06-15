#include"calctime.h"
#include<chrono>

double CalcTime(){
    return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
