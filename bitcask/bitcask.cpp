#include <iostream>
#include "bitcask.h"
#include "Utils/hash.h"
#include "Utils/timeRela.h"
#include "Utils/utils.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dirent.h"
#include <cstddef>
#include <stdlib.h>
#include "messageQ.h"

Bitcask::Bitcask(){

	std::string command = "rm -rf " + this->getTestPath() + "/*";
	system(command.c_str());

    // lock the dir
	int fd = lockFile(testPath + "/" + lockFileName);
	if (-1 == fd) {
		throw std::runtime_error("occupied error.");
    }

	this->setLocker(fd);
	
    this->setDirName(testPath);

	// init hashTable;
	this->hashTable = new HashTable();

    // scan hint file
    std::vector<std::string>* existHintFiles = scanHintFiles();
    parseHintFiles(existHintFiles);

    //get the last file_id
    uint32_t file_id = getLastFileInfo(existHintFiles);
	
    // initial
	this->activeFile = new struct BcFile();
    struct BcFile bf;
	bf.file_offset = 0;
	bf.file_id = file_id;
	bf.fp = -1;
	bf.hintFp = -1;
	this->setActiveFile(&bf);

	//create new active/hint file
	createWriteableFile(this);
	createHintFile(this);
	if (this->getActiveFile()->fp == -1) {
		throw std::runtime_error("check : file pointer is null.");
    }else {
		int fd = this->getActiveFile()->fp;
		int hintFd = this->getActiveFile()->hintFp;
		std::cout<<"fp : "<<fd<<std::endl;
		std::cout<<"hintFp : "<<hintFd<<std::endl;
	}
	this->setBCF(NULL);

	writePID(this->getLocker(), file_id);
    pthread_rwlock_init(&rwlock, NULL);
}

Bitcask::~Bitcask() {
	std::cout<<"~bitcask"<<std::endl;
	if (this->getBCF() != NULL) {
        this->getBCF()->close_BcFiles();
	}	
	std::cout<<"~bitcask ending..."<<std::endl;
	close(this->getActiveFile_fp());
	close(this->getActiveFile_hintFp());
    close(this->getLocker());
//	delete(this->activeFile);
	this->activeFile = NULL;
	std::cout<<"~bitcask ending..."<<std::endl;
//	delete(hashTable);
	hashTable = NULL;
	pthread_rwlock_destroy(&rwlock);  
}

std::string Bitcask::get(std::string key) {
	pthread_rwlock_rdlock(&rwlock);
    std::cout<<"get"<<std::endl;
	Entry *e = this->hashTable->get(key);
	if (e == NULL) {
        pthread_rwlock_unlock(&rwlock);
		return "";
	}

	uint32_t file_id = e->getFileId();
	uint64_t file_offset = e->getFileOffset();
    uint32_t value_size = e->getValueSize();
    uint32_t tstamp = e->getTstamp();

	BcFile *bf = getFileState(file_id);
	if (bf == NULL) {
		pthread_rwlock_unlock(&rwlock);
        std::cout<<"not exists."<<std::endl;
		return NULL;
	}
	
    std::string value = this->getBCF()->readBcFile(bf, this->getDirName(), file_offset, value_size);
	
	pthread_rwlock_unlock(&rwlock);
	return value;
}

void Bitcask::put(const std::string& key, const std::string& value, MessageQueue *cq) {
    pthread_rwlock_wrlock(&rwlock);
	std::cout<<"put"<<std::endl;

	checkActiveFile(this);

    // MessageQueue *cq;
	Entry *e = this->getBCF()->writeBcFile(this->getActiveFile(), key, value, cq);
	
	pthread_rwlock_unlock(&rwlock); 

	this->hashTable->set(key, e);
	
	//while (writeFlag.test_and_set());
	//writeFlag.clear(); 

	return;
}

void Bitcask::del(std::string key) {

	pthread_rwlock_wrlock(&rwlock);
	//Entry *e = this->hashTable.at(key);
	Entry *e = this->hashTable->get(key);
	if (e == NULL) {
        pthread_rwlock_unlock(&rwlock);
        std::cout<<"not exists"<<std::endl;
		return;
	}

    checkActiveFile(this);
    this->getBCF()->delBcFile(this->getActiveFile(), key);

	//this->hashTable.erase(key);
	this->hashTable->set(key, NULL);
    pthread_rwlock_unlock(&rwlock); 
}

void Bitcask::fold(void (*f)(const std::string& key)) {
	pthread_rwlock_rdlock(&rwlock);

//	for_each(begin(), end(), f);
    pthread_rwlock_unlock(&rwlock);
}

void Bitcask::merge() {
	usleep(1000000);
//    sleep(1);
//	while(true) {
	// temporary merged dir
    std::cout<<"merge"<<std::endl;
    char tmpfile[] = "temp-merge";
	std::string command = "mkdir -p " + this->getTestPath() + "/" + tmpfile;
	system(command.c_str());

	// path = testPath + std::string(tmpfile)
	// BcFiles *bcf;         readonly files
	// BcFile *activeFile;   data/hint file
	std::string path = this->getDirName() + "/" + std::string(tmpfile);
	struct BcFile *bf = new struct BcFile;
	BcFiles *bcf = new BcFiles();
	std::string file_id = getCurrentOfSecond();
    auto file_name = path + "/" + file_id + ".data";

    int fd;
    fd = open(file_name.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
    if (fd == -1) {
        throw std::runtime_error("data file open error.");
    }
	
    bf->fp = fd;
    bf->file_id = strtoul(file_id.c_str(), NULL, 10);

	auto file_name_hint = path + "/" + std::to_string(bf->file_id) + ".hint";
	int fd_hint;
    fd_hint = open(file_name_hint.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
    if (fd_hint == -1) {
        throw std::runtime_error("hint file open error.");
    }
    bf->hintFp = fd_hint;
   
	bf->file_offset = 0;
    std::vector<std::string>* existHintFiles = scanHintFiles();
	std::vector<std::pair<std::string, Entry*>> eArray;
    scanEntry(eArray, existHintFiles);
    std::vector<std::pair<std::string, Entry*>>::iterator iter;
    for(iter = eArray.begin(); iter != eArray.end(); iter++) {
		std::string key = iter->first;
		Entry *e_hint = iter->second;
		pthread_rwlock_rdlock(&rwlock);
		Entry *e_hashtable = this->hashTable->get(key);
		if (e_hashtable == NULL) {
			pthread_rwlock_unlock(&rwlock);
			return;
		}
		pthread_rwlock_unlock(&rwlock);
 
		if (!e_hint->isEqual(e_hashtable)) {
			continue;
		}else {
			pthread_rwlock_wrlock(&rwlock);
			Entry *e_hashtable_again = this->hashTable->get(key);
			if (e_hashtable_again == NULL) {
				pthread_rwlock_unlock(&rwlock);
				return;
			}

			if (!e_hint->isEqual(e_hashtable_again)) {
				continue;
			}

			uint32_t file_id_ = e_hint->getFileId();
	        uint64_t file_offset_ = e_hint->getFileOffset();
            uint32_t value_size_ = e_hint->getValueSize();
            uint32_t tstamp_ = e_hint->getTstamp();
		    BcFile *bf_ = this->getFileState(file_id_);

            std::string value = this->getBCF()->readBcFile(bf_, this->getDirName(), file_offset_, value_size_);
			auto offset = bf->file_offset;
			auto logSize = this->getLogSize();
			if (offset >= logSize) {
				close(bf->fp);
				close(bf->hintFp);
				bcf->put_BcFiles(bf, bf->file_id);
			
				std::string file_id = getCurrentOfSecond();
				auto file_name = path + "/" + file_id + ".data";
				int m_fd;
				m_fd = open(file_name.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
           		if (m_fd == -1) {
              	  throw std::runtime_error("hint file open error.");
                }
				bf->fp = m_fd;

				bf->file_id = strtoul(file_id.c_str(), NULL, 10);
				
				auto file_name_hint = path + "/" + std::to_string(bf->file_id) + ".hint";
				int m_fd_hint;
				m_fd_hint = open(file_name_hint.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
            	if (m_fd_hint == -1) {
                	throw std::runtime_error("hint file open error.");
            	}
                bf->hintFp = m_fd_hint;
            }

            MessageQueue *cq = new MessageQueue();
			Entry *entry = bcf->writeBcFile(bf, key, value, cq);

			this->hashTable->set(key, entry);
			delete(bf_);
            delete(cq);
	    }
	}
    
	this->setBCF(bcf);
	this->setActiveFile(bf);
	this->setDirName(path);

	close(fd);
	close(fd_hint);
	delete(bf);
	delete(bcf);
	pthread_rwlock_unlock(&rwlock); 
	
//	sleep(1);
//	}
}

uint64_t Bitcask::getLogSize() {
	return this->LogSize;
}

void Bitcask::setBCF(BcFiles *bcf) {
	if (bcf == NULL) {
        this->bcf = new BcFiles();
	}   
}

BcFiles* Bitcask::getBCF() {
	return this->bcf;
}

void Bitcask::setActiveFile(BcFile *activeFile) {
    this->activeFile->fp = activeFile->fp;
	this->activeFile->file_id = activeFile->file_id;
	this->activeFile->file_offset = activeFile->file_offset;
	this->activeFile->hintFp = activeFile->hintFp;
}

BcFile* Bitcask::getActiveFile() {
	return this->activeFile;
}

int Bitcask::getActiveFile_fp() {
	return this->activeFile->fp;
}

void Bitcask::setActiveFile_fp(int fp) {
	this->activeFile->fp = fp;
}

uint32_t Bitcask::getActiveFile_fileId() {
	return this->activeFile->file_id;
}

void Bitcask::setActiveFile_fileId(uint32_t file_id) {
	this->activeFile->file_id = file_id;
}

uint64_t Bitcask::getActiveFile_offset() {
	return this->activeFile->file_offset;
}

void Bitcask::setActiveFile_offset(uint64_t offset) {
	this->activeFile->file_offset = offset;
}

int Bitcask::getActiveFile_hintFp() {
	return this->activeFile->hintFp;
}

void Bitcask::setActiveFile_hintFp(int hintFp) {
	this->activeFile->hintFp = hintFp;
}

void Bitcask::setDirName(std::string dirName) {
    this->dirName = dirName;
}

std::string Bitcask::getDirName() {
	return this->dirName;
}

std::string Bitcask::getTestPath() {
    return this->testPath;
}

void Bitcask::parseHintFiles(std::vector<std::string>* existHintFiles) {
	std::cout<<"parsing hint files"<<std::endl;	
/**
		tStamp	:	ksz	:	valueSz	:	valuePos	:	key
	    4       :   4   :   4       :       8       :   xxxxx
**/
    std::vector<std::string>::iterator iter;
    char *buffer;
	int HintHeaderSize = 20;
	for (iter=existHintFiles->begin(); iter!=existHintFiles->end();iter++) {
        //read hint file
		std::ifstream file;
		file.open(*iter,std::iostream::in|std::iostream::binary);
		// parse file_id
		uint32_t file_id = strtoul((*iter).c_str(), NULL, 10);
		//hint header size
        while (!file.eof()) {
			// read hint header
			file.read(buffer, HintHeaderSize);

			//decode hint iterm
			uint32_t *tStamp, *ksz, *valueSz;
			uint64_t *file_offset;
			DecodeHintHeader(buffer, tStamp, ksz, valueSz, file_offset);  

			//judge deleted iterm
			if (valueSz == 0) {
				continue;
			}

			//parse key
			char *keyByte;
			file.read(keyByte, *ksz);

			// construct entry
			Entry *e;
			e->setFileId(file_id);
			e->setFileOffset(*file_offset);
			e->setValueSize(*valueSz);
			e->setTstamp(*tStamp);

			//insert hashtable
			//this->hashTable[keyByte] = e;
			this->hashTable->set(keyByte, e);
		}
		file.close();
	}
}


void Bitcask::scanEntry(std::vector<std::pair<std::string, Entry*>> eArray, std::vector<std::string>* existHintFiles) {
    std::vector<std::string>::iterator iter;
    char *buffer;
	int HintHeaderSize = 20;
	int i=0;
	for (iter=existHintFiles->begin(); iter!=existHintFiles->end();iter++) {
		std::ifstream file;
		file.open(*iter,std::iostream::in|std::iostream::binary);
		uint32_t file_id = strtoul((*iter).c_str(), NULL, 10);
        while (!file.eof()) {
			file.read(buffer, HintHeaderSize);
			uint32_t *tStamp, *ksz, *valueSz;
			uint64_t *file_offset;
			DecodeHintHeader(buffer, tStamp, ksz, valueSz, file_offset);  
			if (valueSz == 0) {
				continue;
			}
			char *keyByte;
			file.read(keyByte, *ksz);
		
			Entry *e;
			e->setFileId(file_id);
			e->setFileOffset(*file_offset);
			e->setValueSize(*valueSz);
			e->setTstamp(*tStamp);

            std::pair<std::string, Entry*> ePair{keyByte, e};
			eArray[i++] = ePair;
		}
		file.close();
	}

}


void Bitcask::setLocker(int fd) {
	this->locker = fd;
}

int Bitcask::getLocker() {
	return this->locker;
}

BcFile* Bitcask::getFileState(uint32_t file_id) {
	// active file
    if (this->getActiveFile_fileId() == file_id) {
		return this->getActiveFile();
	}

    // readonly files
	return this->getBCF()->get_BcFiles(file_id);
}