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
	const int circleTimes = 10;
    std::vector<int> keyVector;
	int overwriteRatio = 50;
	for (int i=0; i< circleTimes; i++) {
        keyVector.push_back(getRandomIntRatio(i, overwriteRatio));
	}

	random_shuffle(keyVector.begin(), keyVector.end());

    MessageQueue *cq = new MessageQueue();
	
	for (int i = 0; i < circleTimes; i++) {
		std::cout<<"i: "<<i<<std::endl;
		auto key = keyVector[i];
		auto value = getRandStr(128);
		std::cout<<"xxx"<<std::endl;
		bc.put(std::to_string(key), value, cq);
	}

    delete(cq);
	return;
}

void TestMerge(Bitcask bc) {
	 sleep(1);
     bc.merge();
}


int main()
{

 /*   
	const int threadCount= 1;
	std::thread p[threadCount];
	for (int i = 0; i < threadCount; i++) {
		std::thread a(TestPut);
		p[i] = std::move(a);
        p[i] = std::thread(TestPut);
	}
*/
   
//    static Bitcask bc;
	
/*    const int threadCount= 1;
	std::thread p[threadCount];
	for (int i = 0; i < threadCount; i++) {
		std::thread a(TestMerge, bc);
		p[i] = std::move(a);
        p[i] = std::thread(TestMerge, bc);
	}
	for (int i=0; i<threadCount; i++) 
        p[i].join();
*/		
	TestPut();
/*	std::thread q[threadCount];
	for (int i = 0; i < threadCount; i++) {
		std::thread b(TestMerge, bc);
		q[i] = std::move(b);
        q[i] = std::thread(TestMerge, bc);
	}
*/

    std::cout<<"test ending..."<<std::endl;
	return 0;
}


//	bc.merge();
//  sleep(1);
//  usleep(100000);
//	for (int i = 0; i < threadCount; i++) {
//		p[i].join();
//	}
/*  test messagequeue
    MessageQueue *cq = new MessageQueue();

    #define THREAD_NUM 3
    std::thread threads[THREAD_NUM];

    for ( int i=0; i<THREAD_NUM; ++i )
        threads[i] = std::thread(thread_fun, &cq );

    int i = 100000;
    while( i > 0 )
    {
        Task *pTask = new Task( --i );
        cq->PushTask( pTask );
    }

    for ( int i=0; i<THREAD_NUM; ++i) 
        threads[i].join();
*/
/*
void thread_fun(MessageQueue *arguments) {
	uint64_t data_size = 0;
	std::string data_block;
	int fd;
    while(true) {
        PTask data = arguments->PopTask();
        
        if (data != NULL) {
            std::cout<<"Thread is: "<<std::this_thread::get_id()<<std::endl;
			if (data_size==0) {
                fd = data->fd;
			}
            
			//data->fd, data->offset, data->ch
			if (data_size < 1000000 && fd == data->fd) {
                data_block += data->content;
				data_size += sizeof(data->content);
			} else {
			    write(data->fd, data_block.c_str(), sizeof(data_block));
				data_size = 0;
			}
        }
    }
    return;
}
*/