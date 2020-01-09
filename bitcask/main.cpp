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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <sys/time.h>
#include "Utils/timeRela.h"

//#define CLK_TCK CLOCKS_PER_SEC
//#define CLOCKS_PER_SEC 100000000

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

    MessageQueue *cq = new MessageQueue();

    std::thread gc_thread(&Bitcask::merge, &bc);
/*
    #define THREAD_NUM 3
    std::thread threads[THREAD_NUM];
    for (int i=0; i<THREAD_NUM; ++i ) {
        threads[i] = std::thread(thread_fun, &cq);
	}

    for (int i=0; i<THREAD_NUM; ++i) { 
        threads[i].join();
	}
*/
    int write_fd;
    std::string write_path = "w.txt";
    write_fd = open(write_path.c_str(), O_WRONLY|O_APPEND, S_IRUSR);
    if (write_fd == -1) {
        std::cout<<"write open error"<<std::endl;
    }
    std::cout<<"--------------put----------------"<<std::endl;
	for (int i = 0; i < circleTimes; i++) {
		std::cout<<"i: "<<i<<std::endl;
		auto key = keyVector[i];
        //auto key = i;
		//auto key = 9;
		auto value = getRandStr(128);
    //    clock_t begin = clock();
        uint64_t begin_put = getCurrentOfNanoSecond();
		bc.put(std::to_string(key), value, cq);
    //    clock_t end = clock();
        uint64_t end_put = getCurrentOfNanoSecond();
        uint64_t duration_time = end_put - begin_put;
        std::cout<<"duration time : "<<duration_time<<std::endl;
        std::string write_s = std::to_string(i) + " " + std::to_string(duration_time/100) + "\n";
        write(write_fd, write_s.c_str(), write_s.size());
		std::cout<<"value : "<<value<<std::endl;
		//break;
	}


    //std::thread gc_thread(&Bitcask::merge, &bc);
    int read_fd;
    std::string read_path = "r.txt";
    read_fd = open(read_path.c_str(), O_WRONLY|O_APPEND, S_IRUSR);
    if (read_fd == -1) {
        std::cout<<"read open error"<<std::endl;
    }
    std::cout<<"--------------get----------------"<<std::endl;
    for (int i = 0; i < circleTimes; i++) {
		auto key = keyVector[i];
		//auto key = 9;
        uint64_t begin_get = getCurrentOfNanoSecond();
		auto value = bc.get(std::to_string(key));
        uint64_t end_get = getCurrentOfNanoSecond();
        uint64_t duration_time = end_get - begin_get;
        std::cout<<"duration time : "<<duration_time<<std::endl;
        std::string read_s = std::to_string(i) + " " + std::to_string(duration_time/100) + "\n";
        write(read_fd, read_s.c_str(), read_s.size());
		std::cout<<"value : "<<value<<std::endl;
		//break;
	}
	//bc.get(std::to_string(9));


    std::cout<<"--------------merge 1--------------"<<std::endl;
//    std::thread gc_thread([](Bitcask bc) {
//    bc.merge();
//    }, bc);
//    std::thread gc_thread(&Bitcask::merge, &bc);

    gc_thread.join();
//    bc.merge();
	std::cout<<"--------------merge 2--------------"<<std::endl;
//	  bc.merge();
    delete(cq);
	return;
}

void thread_fun(MessageQueue *arguments ) {   
    int counter = 1000;      // 1000 k/v ==> file
    int data_fd = -1;
    int hint_fd = -1;
    uint64_t data_offset = 0;
    uint64_t hint_offset = 0;
    std::string data_content;
    std::string hint_content;

    while (true) {
        PTask kv_task;
        while (--counter) {
            kv_task = arguments->PopTask();   // lock
            if (kv_task != NULL) {
                std::cout<<"Thread is: "<<std::this_thread::get_id()<<std::endl;
                // splice the k/v
                if (counter=9999) {
                    data_fd = kv_task->data_fd;
                    hint_fd = kv_task->hint_fd;
                    data_offset = kv_task->data_offset;
                    hint_offset = kv_task->hint_offset;
                    data_content = std::string(kv_task->data_content);
                    hint_content = std::string(kv_task->hint_content);
                } else {
                    if (data_fd == kv_task->data_fd && hint_fd == kv_task->hint_fd) {
                        data_content += std::string(kv_task->data_content);
                        hint_content += std::string(kv_task->hint_content);
                    } else {
                        break;
                    }
                }  
            } else {
                break;
            }
        }

        write(data_fd, data_content.c_str(), data_content.size());
        write(hint_fd, hint_content.c_str(), hint_content.size());

        /*
        // range_write
        struct flock fl;
        memset(&fl, 0, sizeof(fl));
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = data_offset;
        fl.l_len = data_content.size();

        if (fcntl(fd, F_SETLK, &fl) == -1) {
            perror("fcntl(F_SETLK)");
            exit(EXIT_FAILURE);
        }
        write(data_fd, data_content.c_str(), data_content.size());

        fl.l_type = F_UNLCK;
        if (fcntl(fd, F_SETLK, &fl) == -1) {
            perror("fcntl(F_SETLK)");
            exit(EXIT_FAILURE);
        }

        struct flock hint_fl;
        memset(&hint_fl, 0, sizeof(fl));
        hint_fl.l_type = F_WRLCK;
        hint_fl.l_whence = SEEK_SET;
        hint_fl.l_start = hint_offset;
        hint_fl.l_len = hint_content.size();

        if (fcntl(hint_fd, F_SETLK, &hint_fl) == -1) {
            perror("fcntl(F_SETLK)");
            exit(EXIT_FAILURE);
        }
        write(hint_fd, hint_content.c_str(), hint_content.size());

        hint_fl.l_type = F_UNLCK;
        if (fcntl(hint_fd, F_SETLK, &hint_fl) == -1) {
            perror("fcntl(F_SETLK)");
            exit(EXIT_FAILURE);
        }

        //Thread end
        if (0 == kv_task->data_content || kv_task == NULL) { 
            break;
        } else {
            delete data;
        }
        */

        arguments->doAction();
        data_fd = -1;
        hint_fd = -1;
        counter = 1000;
    }
    
    return;
}

int main() {	
	TestPut();
    std::cout<<"test ending..."<<std::endl;
	return 0;
}