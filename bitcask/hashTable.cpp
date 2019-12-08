// hashtable.cpp

#include "hashTable.h"
#include <string>
#include <iostream>
#include "entry.h"


HashTable::HashTable() {
    table  = new HashItem*[SIZE]();
	for (int i=0; i<10000; i++) {
        pthread_rwlock_init(&rwlock[i], NULL);
	}	
}

HashTable::~HashTable(){

    for(int i=0; i<SIZE; i++) {
        if(table[i]) {
			delete table[i];
		}
	}

    delete[] table;

    for (int i=0; i<10000; i++) {
        pthread_rwlock_destroy(&rwlock[i]); 
	}
}

void HashTable::set(std::string key, Entry *val){
	std::cout<<"hashtable set()"<<std::endl;
	
	int num = stoi(key, NULL, 10);
    int idx = num % SIZE;
	int flag = num / 1000;

	pthread_rwlock_wrlock(&rwlock[flag]);
    
	// hash conflict
	while (table[idx] && table[idx]->key != key) {
        idx = (idx) %SIZE;
	}

/*	if (table[idx]->key == key) {
        HashItem *item = table[idx];
		std::cout<<"hasdsssshtablllle set()"<<std::endl;
		Entry *tmp;
		tmp = item->entry;
		std::cout<<"has set()"<<std::endl;
		uint32_t tStamp = item->entry->getTstamp();
		if (tStamp > val->getTstamp()) {
    	    return;
		}
		std::cout<<"lle set()"<<std::endl;
	}
*/	
    if(table[idx]) {
		delete table[idx];
		//pthread_rwlock_unlock(&rwlock); 
	}

	if (val == NULL) {
        table[idx] = NULL;
		pthread_rwlock_unlock(&rwlock[flag]); 
		return;     
	}

    if (table == NULL) {
		throw std::runtime_error("hashtable set error.");
        return; 
	}

	if (idx > sizeof(table)/sizeof(HashItem)) {
		table = new HashItem*[1024*1024*1024];
        std::cout<<"idx out of hashtable."<<std::endl;
		//return;
	}

	HashItem *it = new HashItem();
	it->key = key;
	it->entry = new Entry(val->getFileId(), val->getFileOffset(), val->getValueSize(), val->getTstamp());
    table[idx] = it;
    std::cout<<"hashtable set() ending..."<<std::endl;  

    //  std::shared_ptr<HashItem> tmp(it);
    //  std::shared_ptr<HashItem> ppp(table[idx]);
    //	std::swap(tmp, ppp);   
   
	pthread_rwlock_unlock(&rwlock[flag]); 
}
    
Entry* HashTable::get(std::string key){
	
	int num = stoi(key, NULL, 10);
    int idx = num % SIZE;
	int flag = num / 1000;
	pthread_rwlock_rdlock(&rwlock[flag]);
	
    //hash conflict
	while (table[idx] && table[idx]->key != key) {
        idx = (idx) %SIZE;
	}
	Entry *tmp = table[idx]->entry;
	pthread_rwlock_unlock(&rwlock[flag]);
	return tmp;
}

void HashTable::del(std::string key) {
/*	pthread_rwlock_wrlock(&rwlock);
    

	pthread_rwlock_unlock(&rwlock); 
	return;
*/	
}

bool HashTable::setCompare(std::string key, Entry *e) {
/*	pthread_rwlock_wrlock(&rwlock);
    Entry *old = this->get(key);
	if (e->isNewerThan(old)) {
        this->set(key, e);
		return true;
	}

	return false;
    pthread_rwlock_unlock(&rwlock);
*/	 
}
