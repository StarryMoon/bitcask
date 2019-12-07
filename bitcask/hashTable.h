#pragma once
<<<<<<< HEAD
#pragma comment(lib, "pthreadVC2.lib") 
=======
#pragma comment(lib, "pthreadVC2.lib")
//#pragma comment(linker,"/SECTION:.rdata,RW") 
>>>>>>> master

//hashtable

//#include <array>
<<<<<<< HEAD
#include <string>
#include "entry.h"
#include <pthread.h>
//#include <map>  // deduplicate
=======
//#include <map>  // deduplicate
#include <string>
#include "entry.h"
#include <pthread.h>
#include <memory>

>>>>>>> master

struct HashItem {
    std::string key;
    Entry *entry;
<<<<<<< HEAD
	pthread_rwlock_t rwlock;   // item lock         &(HashItem.rwlock)
=======
//	pthread_rwlock_t rwlock;   // item lock         &(HashItem.rwlock)
>>>>>>> master
};

class HashTable
{
<<<<<<< HEAD
    public:

//	      HashTable(int tableSize = 1024*1024*1024);
        
		HashTable();
	    ~HashTable();

		
=======
    public:      
		HashTable();
	    ~HashTable();

>>>>>>> master
		void set(std::string key, Entry *val);
		Entry* get(std::string key);
		void del(std::string key);
		bool setCompare(std::string key, Entry *e);
        

    private:
<<<<<<< HEAD
//	    size_t *table;
        static const int SIZE = 1024*1024*1024;
        HashItem ** table;        // table[i] = HashItem();
        pthread_rwlock_t rwlock;   // range lock
//		pthread_rwlock_t rwlock[10000];
=======
        static const int SIZE = 1024*1024*1024;
        HashItem ** table;                 // table[i] = HashItem();
        pthread_rwlock_t rwlock;  
//		pthread_rwlock_t rwlock[10000];    // range lock
>>>>>>> master
};

