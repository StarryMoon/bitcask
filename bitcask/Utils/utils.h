#pragma once

#include <string>
#include <functional>
#include <limits>
#include <iostream>
#include <chrono>
#include <random>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>    // O_CREAT|O_EXCL
#include "../bitcask.h"

int getRandomInt();

int getRandomIntRatio(int count, int overwriteRatio);

template<typename T>
void inputUntilTrue(T& tobeInput, std::string& msg, std::function<bool(T)> judgeFunc){
	while(!(std::cin>>tobeInput)||!judgeFunc(tobeInput)){
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
		std::cout<<msg;
	}
}

template<typename T,  typename F>
void inputUntilTrue(T& tobeInput, std::string msg, F judgeFunc){
	while(!(std::cin>>tobeInput)||!judgeFunc(tobeInput)){
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
		std::cout<<msg;
	}
}

//generate the str in range of a-z &A-Z
std::string getRandStr(int length);

char* getCrc32(const char* InStr, int len);

void checkActiveFile(Bitcask *bc);

void createWriteableFile(Bitcask *bc); 

void createHintFile(Bitcask *bc); 

std::vector<std::string>* scanHintFiles();

std::vector<std::string>* listDataFiles();

uint32_t getLastFileInfo(std::vector<std::string>*);

int lockFile(std::string path);

void writePID(int fd, uint32_t file_id);