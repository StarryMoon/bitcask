// main.cpp
#include <iostream>
#include "bitcask.h"
#include "Utils/utils.h"
#include <thread>
#include <unistd.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <vector>
#include "messageQ.h"
#include <string>

void TestPut() {
	std::cout<<"TestPut()"<<std::endl;
	static Bitcask bc;
	const int circleTimes = 10000;
    std::vector<int> keyVector;
	int overwriteRatio = 50;   // --> 50%
	for (int i=0; i< circleTimes; i++) {
        keyVector.push_back(getRandomIntRatio(i, overwriteRatio));
	}

	random_shuffle(keyVector.begin(), keyVector.end());

//    MessageQueue *cq = new MessageQueue();
/*
	std::thread gc_thread([](Bitcask bc) {
        bc.merge();
    }, bc);
	gc_thread.join();
*/
    
    std::cout<<"--------------put----------------"<<std::endl;
	for (int i = 0; i < circleTimes; i++) {
		std::cout<<"i: "<<i<<std::endl;
		auto key = keyVector[i];
		//auto key = 9;
		auto value = getRandStr(128);
		bc.put(std::to_string(key), value);
		std::cout<<"value : "<<value<<std::endl;
		//break;
	}
/*    std::cout<<"--------------get----------------"<<std::endl;
    for (int i = 0; i < circleTimes; i++) {
		auto key = keyVector[i];
		//auto key = 9;
		auto value = bc.get(std::to_string(key));
		std::cout<<"value : "<<value<<std::endl;
		//break;
	}
	//bc.get(std::to_string(9));
*/
    std::cout<<"--------------merge 1--------------"<<std::endl;
    bc.merge();
	std::cout<<"--------------merge 2--------------"<<std::endl;
	bc.merge();
//    delete(cq);
	return;
}


int main()
{
		
	TestPut();
    std::cout<<"test ending..."<<std::endl;
	return 0;
}
