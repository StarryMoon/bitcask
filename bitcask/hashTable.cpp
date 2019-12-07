// hashtable.cpp

#include "hashTable.h"
#include <string>

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
            if(table[i]) delete table[i];
	}

    delete[] table;

	pthread_rwlock_destroy(&rwlock); 
}

void HashTable::set(std::string key, Entry *val){
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
