#include"../pch.h"
#include "timeRela.h"
#include <time.h>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

std::string getStrToday(){
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream  ss;
	ss<< std::put_time(std::localtime(&t), "%F") ;
    return std::string(ss.str());
}

std::string getCurrentOfFormat(const char* format) {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream  ss;
	tm buf;
	try {
		ss<< std::put_time(std::localtime(&t), "%F %T") ;
	}catch (std::exception e) {
		throw std::runtime_error("format error");
	}

    return std::string(ss.str());
}


std::string getCurrentOfSecond() {
	time_t nowTime;
	nowTime = time(NULL);
    
	return std::to_string(nowTime);
}

uint64_t getCurrentOfMicroSecond() {

	struct timeval tv;
    gettimeofday(&tv,NULL);

	uint64_t timeStamp = 1000000 * tv.tv_sec + tv.tv_usec;

    return timeStamp;
}


