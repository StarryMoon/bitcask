#pragma once
#pragma comment(lib, "pthreadVC2.lib") 

//hashtable

//#include <array>
#include <string>
#include "entry.h"
#include <pthread.h>
//#include <map>  // deduplicate

struct HashItem {
    std::string key;
    Entry *entry;
	pthread_rwlock_t rwlock;   // item lock         &(HashItem.rwlock)
};

class HashTable
{
    public:

//	      HashTable(int tableSize = 1024*1024*1024);
        
		HashTable();
	    ~HashTable();

		
		void set(std::string key, Entry *val);
		Entry* get(std::string key);
		void del(std::string key);
		bool setCompare(std::string key, Entry *e);
        

    private:
//	    size_t *table;
        static const int SIZE = 1024*1024*1024;
        HashItem ** table;        // table[i] = HashItem();
        pthread_rwlock_t rwlock;   // range lock
//		pthread_rwlock_t rwlock[10000];
};

