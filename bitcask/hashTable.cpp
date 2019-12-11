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
	
	std::cout<<"hash set : "<<std::endl;

	if (table == NULL) {
		throw std::runtime_error("hashtable set error.");
        return; 
	}

	int num = stoi(key, NULL, 10);
    int idx = num % SIZE;
	int flag = num / 1000;

    std::cout<<"hash set idx : "<<idx<<std::endl;
//	std::cout<<"hash set size : "<<sizeof(table)<<std::endl;

/*	if (idx >= sizeof(table)/sizeof(HashItem)) {
		throw std::runtime_error("hashtable out of stack error.");
		table = new HashItem*[1024*1024*1024]();
		//return;
	}
*/
	pthread_rwlock_wrlock(&rwlock[flag]);
    
	// hash conflict
	while (this->table[idx] && (table[idx]->key != key)) {
        idx = (idx+1) %SIZE;
	}

    std::cout<<"hash key : "<<std::endl;
	if (this->table[idx] && this->table[idx]->key == key) {
        HashItem *item = table[idx];
		Entry *tmp;
		tmp = item->entry;
		uint32_t tStamp = item->entry->getTstamp();
		if (tStamp > val->getTstamp()) {
    	    return;
		}
	}
	
    if(table[idx]) {
		delete table[idx];
		//pthread_rwlock_unlock(&rwlock); 
	}

	if (val == NULL) {
        table[idx] = NULL;
		pthread_rwlock_unlock(&rwlock[flag]); 
		return;     
	}

	HashItem *it = new HashItem();
	it->key = key;
	it->entry = new Entry(val->getFileId(), val->getFileOffset(), val->getValueSize(), val->getTstamp());
    table[idx] = it;

    //  std::shared_ptr<HashItem> tmp(it);
    //  std::shared_ptr<HashItem> ppp(table[idx]);
    //	std::swap(tmp, ppp);   
   
	pthread_rwlock_unlock(&rwlock[flag]); 
}
    
Entry* HashTable::get(std::string key){
	std::cout<<"hash get : "<<std::endl;
	int num = stoi(key, NULL, 10);
    int idx = num % SIZE;
	int flag = num / 1000;
	pthread_rwlock_rdlock(&rwlock[flag]);
	
    //hash conflict
	while (table[idx] && table[idx]->key != key) {
        idx = (idx+1) %SIZE;
	}

	HashItem *it = table[idx];
	if (it == NULL) {
        return NULL;
	}

	Entry *tmp = it->entry;
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
