#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

#include<sched.h>

class CommonUtils
{
public:

    static long getTimeInMicros()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_usec + tv.tv_sec * 1000000L;

    }
    
    static void setAffinity(const int cpu_id)
    {
        cpu_set_t cpu_set;
        CPU_ZERO(&cpu_set);
        CPU_SET(cpu_id, &cpu_set);
        //CPU_SET(0, &cpu_set);
        sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set);
    }
    
    static void nanoSleepUtil(const int nanos)
    {
        struct timespec tim, tim2;
        tim.tv_sec = 0;
        tim.tv_nsec = nanos;
        
        nanosleep(&tim, &tim2);
   }

    template <typename T>
    static void str_to_T(const char * str, T& val) {
        while (*str) {
            val = val * 10 + (*str++ -'0');
        }
    }

    static const char* today() {
        time_t t = time(0); // get time now
        struct tm * now = localtime(& t);
        static char dateNow[64];
        sprintf(dateNow, "%04d%02d%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
        return dateNow;
    }

    std::string CreateTodayFolder(const std::string& folderPath) {

        struct stat st = {0};
        std::string todayFolder = folderPath + std::string(CommonUtils::today());
        if (stat(todayFolder.c_str(), &st) == -1) {
            mkdir(todayFolder.c_str(), 0700);
        }
        return todayFolder;
    }
};

#endif	/* COMMON_UTILS_H */

