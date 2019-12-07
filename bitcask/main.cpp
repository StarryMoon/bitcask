// main.cpp
#include <iostream>
#include "bitcask.h"
#include "Utils/utils.h"
#include <thread>
#include <unistd.h>
#include <vector>

void TestPut() {
	std::cout<<"TestPut()"<<std::endl;
	static Bitcask bc;

	const int circleTimes = 10;
    std::vector<int> keyVector;
	int overwriteRatio = 50;
	for (int i=0; i< circleTimes; i++) {
        keyVector.push_back(getRandomIntRatio(i, overwriteRatio));
	}

	random_shuffle(keyVector.begin(), keyVector.end());

	for (int i = 0; i < circleTimes; i++) {

		std::cout<<"i: "<<i<<std::endl;
		//auto key = getRandStr(8);
		//auto key = getRandomInt();
		auto key = keyVector[i];
		auto value = getRandStr(128);
		bc.put(std::to_string(key), value);
	}
	bc.merge();
//  sleep(1);
//  usleep(100000);
	return;
	
}


int main()
{
	//test multi-thread 
/*	const int threadCount= 1;
	std::thread p[threadCount];

	for (int i = 0; i < threadCount; i++) {

		std::thread a(TestPut);
		p[i] = std::move(a);
        p[i] = std::thread(TestPut);
	}

	for (int i = 0; i < threadCount; i++) {
		p[i].join();
	}
*/
    TestPut();
    std::cout<<"test ending... "<<std::endl;
	return 0;
}

