//#pragma pack(1)
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
#include <sstream>

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
    std::vector<std::string>* existHintFiles;
//	scanHintFiles(existHintFiles);
//    parseHintFiles(existHintFiles);

    //get the last file_id
    uint64_t file_id = getLastFileInfo(existHintFiles);
	
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
	}
	this->setBCF(NULL);

	writePID(this->getLocker(), file_id);
    pthread_rwlock_init(&rwlock, NULL);
}

Bitcask::~Bitcask() {
	if (this->getBCF() != NULL) {
        this->getBCF()->close_BcFiles();
	}	
	close(this->getActiveFile_fp());
	close(this->getActiveFile_hintFp());
    close(this->getLocker());
//	delete(this->activeFile);
	this->activeFile = NULL;
//	delete(hashTable);
	hashTable = NULL;
	pthread_rwlock_destroy(&rwlock);  
}

std::string Bitcask::get(std::string key) {
	pthread_rwlock_rdlock(&rwlock);
	
	Entry *e = this->hashTable->get(key);
	if (e == NULL) {
        pthread_rwlock_unlock(&rwlock);
		return "";
	}

	uint64_t file_id = e->getFileId();
	uint64_t file_offset = e->getFileOffset();
    uint32_t value_size = e->getValueSize();
    uint64_t tstamp = e->getTstamp();

	BcFile *bf = getFileState(file_id);
	if (bf == NULL) {
		pthread_rwlock_unlock(&rwlock);
        std::cout<<"not exists."<<std::endl;
		return NULL;
	}

    std::string value = this->getBCF()->readBcFile(bf, this->getDirName(), file_offset, value_size);
	
	std::cout<<"value : "<<value<<std::endl;

	pthread_rwlock_unlock(&rwlock);
	return value;
}

void Bitcask::put(const std::string& key, const std::string& value, MessageQueue *cq) {
    pthread_rwlock_wrlock(&rwlock);

	checkActiveFile(this);

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

    std::vector<std::string> *existHintFiles = new std::vector<std::string>();
	scanHintFiles(existHintFiles);
	std::vector<std::pair<std::string, Entry*>> eArray;
    eArray = scanEntry(existHintFiles);
	std::cout<<"scan ending..."<<std::endl;
    std::cout<<"array size()..."<<eArray.size()<<std::endl;

//  char tmpfile[] = "temp-merge";
//	std::string command = "mkdir -p " + this->getTestPath() + "/" + tmpfile;
//	system(command.c_str());

	// path = testPath + std::string(tmpfile)
	// BcFiles *bcf;         readonly files
	// BcFile *activeFile;   data/hint file
	//std::string path = this->getDirName() + "/" + std::string(tmpfile);
	std::string path = this->getDirName();
	struct BcFile *bf = new struct BcFile;
	BcFiles *bcf = new BcFiles();
	std::string file_id = std::to_string(getCurrentOfMicroSecond());
    auto file_name = path + "/" + file_id + ".data";

    int fd;
    fd = open(file_name.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
    if (fd == -1) {
        throw std::runtime_error("data file open error.");
    }
	
    bf->fp = fd;
    bf->file_id = strtoull(file_id.c_str(), NULL, 10);

	auto file_name_hint = path + "/" + std::to_string(bf->file_id) + ".hint";
	int fd_hint;
    fd_hint = open(file_name_hint.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
    if (fd_hint == -1) {
        throw std::runtime_error("hint file open error.");
    }
    bf->hintFp = fd_hint;
   
	bf->file_offset = 0;


    std::vector<std::pair<std::string, Entry*>>::iterator iter;
    for(iter = eArray.begin(); iter != eArray.end(); iter++) {
		std::string key = iter->first;
		std::cout<<"insert key : "<<key<<std::endl;
		Entry *e_hint = iter->second;
		pthread_rwlock_rdlock(&rwlock);
		Entry *e_hashtable = this->hashTable->get(key);
		if (e_hashtable == NULL) {
			pthread_rwlock_unlock(&rwlock);
			return;
		}
		pthread_rwlock_unlock(&rwlock);
 
		if (!e_hint->isEqual(e_hashtable)) {    // false
			continue;
		}else {
			//read value first

			pthread_rwlock_wrlock(&rwlock);
			Entry *e_hashtable_again = this->hashTable->get(key);
			if (e_hashtable_again == NULL) {
				pthread_rwlock_unlock(&rwlock);
				return;
			}

			if (!e_hint->isEqual(e_hashtable_again)) {
				pthread_rwlock_unlock(&rwlock);
				continue;
			}

			uint64_t file_id_ = e_hint->getFileId();
	        uint64_t file_offset_ = e_hint->getFileOffset();
            uint32_t value_size_ = e_hint->getValueSize();
            uint64_t tstamp_ = e_hint->getTstamp();
		    BcFile *bf_ = this->getFileState(file_id_);

            std::cout<<"val : "<<value_size_<<std::endl;
            std::string value = this->getBCF()->readBcFile(bf_, this->getDirName(), file_offset_, value_size_);
			std::cout<<"merge value : "<<value<<std::endl;
			std::cout<<"merge value size : "<<value.size()<<std::endl;
			auto offset = bf->file_offset;
			auto logSize = this->getLogSize();
			if (offset >= logSize) {
				close(bf->fp);
				close(bf->hintFp);
				bcf->put_BcFiles(bf, bf->file_id);
			
				std::string file_id = std::to_string(getCurrentOfMicroSecond());
				auto file_name = path + "/" + file_id + ".data";
				int m_fd;
				m_fd = open(file_name.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
           		if (m_fd == -1) {
              	  throw std::runtime_error("hint file open error.");
                }
				bf->fp = m_fd;

				bf->file_id = strtoull(file_id.c_str(), NULL, 10);
				
				auto file_name_hint = path + "/" + std::to_string(bf->file_id) + ".hint";
				int m_fd_hint;
				m_fd_hint = open(file_name_hint.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
            	if (m_fd_hint == -1) {
                	throw std::runtime_error("hint file open error.");
            	}
                bf->hintFp = m_fd_hint;
            }

            MessageQueue *cq = NULL;
			Entry *entry = bcf->writeBcFile(bf, key, value, cq);
            pthread_rwlock_unlock(&rwlock);

            std::cout<<"merge hash : "<<std::endl;
			this->hashTable->set(key, entry);
			//delete(bf_);
			std::cout<<"merge hash"<<std::endl;
	    }
	}
    
	std::cout<<"merge."<<std::endl;

	this->setBCF(bcf);
	this->setActiveFile(bf);
	this->setDirName(path);

	close(fd);
	close(fd_hint);
	delete(bf);
	delete(bcf);
	//pthread_rwlock_unlock(&rwlock); 

	// delete old files
	//std::vector<std::string> *existHintFiles = new std::vector<std::string>();
	std::vector<std::string>::iterator iter_delete_hint;
	std::cout<<"entry : "<<existHintFiles->size()<<std::endl;
	for (iter_delete_hint=existHintFiles->begin(); iter_delete_hint!=existHintFiles->end(); iter_delete_hint++) {
        uint64_t remove_file_id = strtoull((*iter_delete_hint).c_str(), NULL, 10);
		std::cout<<"file name : "<<remove_file_id<<std::endl;
		std::string path = this->getDirName() + "/";

//std::cout<<"not delete..."<<std::endl;

	    //std::string command = "rm -rf " + path;
		std::string delete_data_command = "rm -f " + path + std::to_string(remove_file_id) + ".data" ;
	    system(delete_data_command.c_str());
		std::string delete_hint_command = "rm -f " + path + std::to_string(remove_file_id) + ".hint" ;
	    system(delete_hint_command.c_str());		
	}

	
	std::cout<<"merge..."<<std::endl;
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

uint64_t Bitcask::getActiveFile_fileId() {
	return this->activeFile->file_id;
}

void Bitcask::setActiveFile_fileId(uint64_t file_id) {
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
//	std::cout<<"parsing hint files"<<std::endl;	
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


std::vector<std::pair<std::string, Entry*>> Bitcask::scanEntry(std::vector<std::string>* existHintFiles) {
    std::vector<std::string>::iterator iter;
	std::cout<<"entry : "<<existHintFiles->size()<<std::endl;
	std::vector<std::pair<std::string, Entry*>> eArray;
    //char *buffer;
	
	int HintHeaderSize = 24;
	for (iter=existHintFiles->begin(); iter!=existHintFiles->end();iter++) {
		std::ifstream file;
		file.open(this->getDirName() + "/" + *iter, std::iostream::in|std::iostream::binary);
		std::cout<<"dir name : "<<this->getDirName()<<std::endl;
		std::cout<<"iter : "<<*iter<<std::endl;
		uint64_t file_id = strtoull((*iter).c_str(), NULL, 10);

        //while (!file.eof()) {
		while (file.peek() != EOF) {
			char *bufer;
		    bufer = new char[24];
            file.read(bufer, 24);
//			int readedytes = file.gcount();
//			std::cout<<"read : "<<readedytes<<std::endl;

            // shift count >= width of type
            uint32_t byte0 = (unsigned char)(bufer[0]) << 24;
			uint32_t byte1 = (unsigned char)(bufer[1]) << 16;
			uint32_t byte2 = (unsigned char)(bufer[2]) << 8;
			uint32_t byte3 = (unsigned char)(bufer[3]);
			uint32_t t_high = byte0|byte1|byte2|byte3;
			std::stringstream ss_high;
			ss_high<<std::hex<<t_high;

			uint32_t byte4 = (unsigned char)(bufer[4]) << 24;
			uint32_t byte5 = (unsigned char)(bufer[5]) << 16;
			uint32_t byte6 = (unsigned char)(bufer[6]) << 8;
			uint32_t byte7 = (unsigned char)(bufer[7]);
            uint32_t t_low = byte4|byte5|byte6|byte7;
			std::stringstream ss_low;
			ss_low<<std::hex<<t_low;
            
			std::string t_tmp = ss_high.str() + ss_low.str();
			std::stringstream ss;
			ss.str(t_tmp);
			uint64_t ts;
			ss>>std::hex>>ts;

			uint32_t b0 = bufer[8] << 24;
			uint32_t b1 = bufer[9] << 16;
			uint32_t b2 = bufer[10] << 8;
			uint32_t b3 = bufer[11];
			uint32_t kSz = b0|b1|b2|b3;

			uint32_t v0 = (unsigned char)(bufer[12]) << 24;
			uint32_t v1 = (unsigned char)(bufer[13]) << 16;
			uint32_t v2 = (unsigned char)(bufer[14]) << 8;
			uint32_t v3 = (unsigned char)(bufer[15]);
			uint32_t vSz = v0|v1|v2|v3;

			uint32_t offset_0 = (unsigned char)(bufer[16]) << 24;
			uint32_t offset_1 = (unsigned char)(bufer[17]) << 16;
			uint32_t offset_2 = (unsigned char)(bufer[18]) << 8;
			uint32_t offset_3 = (unsigned char)(bufer[19]);
			uint32_t offset_high = offset_0|offset_1|offset_2|offset_3;
			std::stringstream ss_offset_high;
			ss_offset_high<<std::hex<<offset_high;

			uint32_t offset_4 = (unsigned char)(bufer[20]) << 24;
			uint32_t offset_5 = (unsigned char)(bufer[21]) << 16;
			uint32_t offset_6 = (unsigned char)(bufer[22]) << 8;
			uint32_t offset_7 = (unsigned char)(bufer[23]);
            uint64_t offset_low = offset_4|offset_5|offset_6|offset_7;
			std::stringstream ss_offset_low;
			ss_offset_low<<std::hex<<offset_low;

			std::string offset_tmp = ss_offset_high.str() + ss_offset_low.str();
			std::stringstream ss_offset;
			ss_offset.str(offset_tmp);
			uint64_t offset;
			ss_offset>>std::hex>>offset;

			if (vSz == 0) {
				continue;
			}

			//char *keyByte;         // not allocated memory
			char *keyByte = new char[kSz];
			file.read(keyByte, kSz);     // __memcpy_avx_unaligned()
			std::cout<<"key : "<<keyByte<<std::endl;
				
			Entry *e = new Entry(file_id, offset, vSz, ts);
			e->setFileId(file_id);
			e->setFileOffset(offset);
			e->setValueSize(vSz);
			e->setTstamp(ts);

            std::pair<std::string, Entry*> ePair{keyByte, e};
			//eArray[i++] = ePair;
			eArray.push_back(ePair);
		}
		file.close();	
	}

	return eArray;
}


void Bitcask::setLocker(int fd) {
	this->locker = fd;
}

int Bitcask::getLocker() {
	return this->locker;
}

BcFile* Bitcask::getFileState(uint64_t file_id) {
	// active file
    if (this->getActiveFile_fileId() == file_id) {
		return this->getActiveFile();
	}

    // readonly files
	return this->getBCF()->get_BcFiles(file_id);
}