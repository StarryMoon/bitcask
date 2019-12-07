#pragma once
#pragma comment(lib, "pthreadVC2.lib") 

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <atomic>
#include <string>
#include "Utils/hash.h"
#include "Utils/timeRela.h"
#include <pthread.h>
#include "entry.h"
#include "Utils/utils.h"
#include <stdlib.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

    BcFiles::BcFiles() {
		pthread_rwlock_init(&rwlock, NULL);
	}

	BcFiles::~BcFiles() {
		this->close_BcFiles();
		pthread_rwlock_destroy(&rwlock);  
	}
 
    BcFile* BcFiles::get_BcFiles(uint32_t file_id) {
		pthread_rwlock_rdlock(&rwlock);
        std::cout<<"get bcfiles"<<std::endl;
        if(this->bfs.count(file_id) > 0) {
            return this->bfs[file_id];
		} else {
			return NULL;
		}
	    pthread_rwlock_unlock(&rwlock);
	}

	void BcFiles::put_BcFiles(BcFile *bcf, uint32_t file_id) {
		pthread_rwlock_wrlock(&rwlock);
        this->bfs.insert(std::pair<uint32_t, BcFile*>(file_id, bcf));
		pthread_rwlock_unlock(&rwlock); 
	}

	void BcFiles::close_BcFiles() {
		pthread_rwlock_rdlock(&rwlock);
        std::map<uint32_t, BcFile*> bcf = this->getBFS();
        if (bcf.size() == 0) {
            return;
		}

        std::map<uint32_t, BcFile*>::iterator iter;
		iter = bcf.begin();
		while(iter != bcf.end()) {
			std::cout<<"close bcfiles..."<<std::endl;
		    BcFile *bf = iter->second;
            close(bf->fp);
			close(bf->hintFp);
		    iter++;
	    }
		pthread_rwlock_unlock(&rwlock);
	}

    void BcFiles::newBcFile(BcFile* bcf, std::ofstream fp, uint32_t file_id, uint64_t file_offset, std::ofstream hintFp) {
//		BcFile bcf = {fp, std::to_string(file_id), file_offset, hintFp};
//		return &bcf;
        return;
	}

	void BcFiles::openBcFile(BcFile* bcf, std::string dirName, std::string tStamp) {
        std::string file_name = dirName + "/" + tStamp + ".data";
		int fd;
        fd = open(file_name.c_str(), O_WRONLY|O_APPEND, S_IRUSR);
        if (fd == -1) {
            throw std::runtime_error("hint file open error.");
        }	
        bcf->fp = fd;
        return;
	}

	std::string BcFiles::readBcFile(BcFile *bf, std::string dirName, uint64_t offset, uint32_t len) {
        std::ifstream in;
		in.open(dirName + "/" + std::to_string(bf->file_id) + ".data", std::iostream::in);
        in.seekg(offset, std::iostream::beg);
		
		char *buffer;
		buffer = new char[len];
        in.read(buffer, len);
        in.close();

		//TODO:; decode / crc32 check

        std::string s;
		for (int i=0; i<len; i++) {
            s += std::to_string(buffer[i]);
		}
		
		delete(buffer);
		return s;
	}

	Entry* BcFiles::writeBcFile(BcFile *bf, const std::string& key, const std::string& value) {

		std::cout<<"write bcfile"<<std::endl;

//		bf->fp = 4;
		std::cout<<"fppp : "<<bf->fp<<std::endl;

		auto timestamp = getCurrentOfSecond();
	    auto keySize = std::to_string(key.size());
	    auto valueSize = std::to_string(value.size());
	    //auto hashKey = BKDRHash(key.c_str());

        // HeaderSize = 16
	    auto valueOffset = bf->file_offset + HeaderSize + strtoull(keySize.c_str(), NULL, 10);
		std::cout<<"valueOffset : "<<valueOffset<<std::endl;
	    std::string strData = timestamp + keySize + valueSize + key + value;

		std::cout<<"cstr : "<<strData.c_str()<<std::endl;
		char* crc32 = getCrc32(strData.c_str(), strData.size());      // char*
		std::cout<<"crc32 : "<<crc32<<std::endl;

		//char *ch = "ch";
		char ch[1000];
		EncodeData(ch, strtoul(crc32, NULL, 10), strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), key, value); 

		std::cout<<"fp : "<<bf->fp<<std::endl;
		//write(bf->fp, ch, std::to_string(ch).size());
//		strData = crc32 + strData;
		write(bf->fp, ch, sizeof(ch));

		//std::string strHint = timestamp + keySize + valueSize + std::to_string(valueOffset) + key;
        //bf->hintFp << strHint;
		char chHint[1000];
		EncodeHint(chHint, strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), valueOffset, key);
		
		write(bf->hintFp, chHint, sizeof(chHint));
		std::cout<<"hintFp : "<<bf->hintFp<<std::endl;

        //HeaderSize =16
		bf->file_offset = bf->file_offset + HeaderSize + strtoull(keySize.c_str(), NULL, 10) + strtoull(valueSize.c_str(), NULL, 10);

        std::cout<<"write bcfile ending..."<<std::endl;
//        bf->file_id = 0;

        return new Entry(bf->file_id, valueOffset, strtoul(valueSize.c_str(), NULL, 10), strtoul(timestamp.c_str(), NULL, 10));
	}

    // tombstone : keySize=0, valueSize=0
	void BcFiles::delBcFile(BcFile *bf, const std::string& key) {

		auto timestamp = getCurrentOfSecond();            // std::string
	    auto keySize = std::to_string(0);
	    auto valueSize = std::to_string(0);  // 0
	    //auto hashKey = BKDRHash(key.c_str());

        auto valueOffset = bf->file_offset + HeaderSize + strtoull(keySize.c_str(), NULL, 10);
	    std::string strData = timestamp + keySize + valueSize + key;
		//char *ch = getCrc32(strData.c_str(), strData.size());
		char* crc32 = getCrc32(strData.c_str(), strData.size());      // char*
        
		char ch[1000];
		EncodeData(ch, strtoul(crc32, NULL, 10), strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), key, NULL); 
		write(bf->fp, ch, sizeof(ch));
	   
	    //std::string strHint = timestamp + keySize + valueSize + std::to_string(valueOffset) + key;
		char chHint[1000];
		EncodeHint(chHint, strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), valueOffset, key);

		write(bf->hintFp, chHint, sizeof(chHint));
        
		// HeaderSize 16
		bf->file_offset = bf->file_offset + HeaderSize + strtoull(keySize.c_str(), NULL, 10) + strtoull(valueSize.c_str(), NULL, 10);
		
		return;
	}
   
    void BcFiles::setBFS(std::map<uint32_t, BcFile*> bfs) {
		this->bfs = bfs;
	}

	std::map<uint32_t, BcFile*> BcFiles::getBFS() {
		return this->bfs;
	}


