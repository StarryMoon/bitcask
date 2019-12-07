#pragma once
#pragma comment(lib, "pthreadVC2.lib")
//#pragma comment(linker,"/SECTION:.rdata,RW") 

//hashtable
//#include <map>  // deduplicate

#include <string>
#include "entry.h"
#include <pthread.h>
#include <memory>


struct HashItem {
    std::string key;
    Entry *entry;
//	pthread_rwlock_t rwlock;   // item lock         &(HashItem.rwlock)
};

class HashTable
{
    public:      
		HashTable();
	    ~HashTable();

		void set(std::string key, Entry *val);
		Entry* get(std::string key);
		void del(std::string key);
		bool setCompare(std::string key, Entry *e);
        
    private:
        static const int SIZE = 1024*1024*1024;
        HashItem ** table;                 // table[i] = HashItem();
        pthread_rwlock_t rwlock;  
//		pthread_rwlock_t rwlock[10000];    // range lock
};

