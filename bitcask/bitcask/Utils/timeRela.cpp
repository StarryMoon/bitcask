#include"../pch.h"
#include "timeRela.h"
#include <time.h>
#include <sstream>
#include <stdexcept>

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
		//localtime is not a thread-safe function
		//localtime_s(&buf, &t);
		//ss<< std::put_time(&buf,"%F %T") ;

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