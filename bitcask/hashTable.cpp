// hashtable.cpp

#include "hashTable.h"
#include <string>
<<<<<<< HEAD
=======
#include <iostream>
#include "entry.h"
>>>>>>> master

/*
HashTable::HashTable(int tableSize){
	table = new size_t[tableSize]();
}
*/

HashTable::HashTable() {
    table  = new HashItem*[SIZE]();
	pthread_rwlock_init(&rwlock, NULL);
}

HashTable::~HashTable(){

    for(int i=0; i<SIZE; i++) {
<<<<<<< HEAD
            if(table[i]) delete table[i];
=======
        if(table[i]) {
//			delete table[i]->entry;
			delete table[i];
		}
>>>>>>> master
	}

    delete[] table;

	pthread_rwlock_destroy(&rwlock); 
}

void HashTable::set(std::string key, Entry *val){
<<<<<<< HEAD
	pthread_rwlock_wrlock(&rwlock);
    int idx = stoi(key, NULL, 10) % SIZE;
    if(table[idx]) delete table[idx];
	struct HashItem it;
	it.key = key;
	it.entry->setFileId(val->getFileId());
	it.entry->setFileOffset(val->getFileOffset());
	it.entry->setValueSize(val->getValueSize());
	it.entry->setTstamp(val->getTstamp());
    table[idx] = &it;
=======
	std::cout<<"hashtable set()"<<std::endl;
	pthread_rwlock_wrlock(&rwlock);
    int idx = stoi(key, NULL, 10) % SIZE;
    if(table[idx]) {
		delete table[idx];
		//pthread_rwlock_unlock(&rwlock); 
	}

	if (val == NULL) {
        table[idx] = NULL;
		pthread_rwlock_unlock(&rwlock); 
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
//    if (table[idx] != NULL) {
//		*(table[idx]) = it;
//        table[idx] = it;
//	}
//     table[idx] = &it;  

//  std::shared_ptr<HashItem> tmp(it);
//  std::shared_ptr<HashItem> ppp(table[idx]);
//	std::swap(tmp, ppp);   
std::cout<<"hashtable set() endddding..."<<std::endl;   
>>>>>>> master
	pthread_rwlock_unlock(&rwlock); 
}
    
Entry* HashTable::get(std::string key){
	pthread_rwlock_rdlock(&rwlock);
    int idx = stoi(key, NULL, 10) %SIZE;
    //return table[idx] ? table[idx]->getVal() : -1;
	Entry *tmp = table[idx]->entry;
	pthread_rwlock_unlock(&rwlock);
	return tmp;
}

void HashTable::del(std::string key) {
	pthread_rwlock_wrlock(&rwlock);
    
	// ...

	pthread_rwlock_unlock(&rwlock); 
	return;
}

bool HashTable::setCompare(std::string key, Entry *e) {
	pthread_rwlock_wrlock(&rwlock);
    Entry *old = this->get(key);
	if (e->isNewerThan(old)) {
        this->set(key, e);
		return true;
	}

	return false;
    pthread_rwlock_unlock(&rwlock); 
}
