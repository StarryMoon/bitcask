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
#include "messageQ.h"

    BcFiles::BcFiles() {
		pthread_rwlock_init(&rwlock, NULL);
	}

	BcFiles::~BcFiles() {
		this->close_BcFiles();
		pthread_rwlock_destroy(&rwlock);  
	}
 
    BcFile* BcFiles::get_BcFiles(uint64_t file_id) {
		pthread_rwlock_rdlock(&rwlock);
        if(this->bfs.count(file_id) > 0) {
            return this->bfs[file_id];
		} else {
			return NULL;
		}
	    pthread_rwlock_unlock(&rwlock);
	}

	void BcFiles::put_BcFiles(BcFile *bcf, uint64_t file_id) {
		pthread_rwlock_wrlock(&rwlock);
        this->bfs.insert(std::pair<uint64_t, BcFile*>(file_id, bcf));
		pthread_rwlock_unlock(&rwlock); 
	}

	void BcFiles::close_BcFiles() {
		pthread_rwlock_rdlock(&rwlock);
        std::map<uint64_t, BcFile*> bcf = this->getBFS();
        if (bcf.size() == 0) {
            return;
		}

        std::map<uint64_t, BcFile*>::iterator iter;
		iter = bcf.begin();
		while(iter != bcf.end()) {
		    BcFile *bf = iter->second;
            close(bf->fp);
			close(bf->hintFp);
		    iter++;
	    }
		pthread_rwlock_unlock(&rwlock);
	}

    void BcFiles::newBcFile(BcFile* bcf, std::ofstream fp, uint64_t file_id, uint64_t file_offset, std::ofstream hintFp) {
//		BcFile bcf = {fp, std::to_string(file_id), file_offset, hintFp};
//		return &bcf;
        return;
	}

	void BcFiles::openBcFile(BcFile* bcf, std::string dirName, uint64_t tStamp) {
        std::string file_name = dirName + "/" + std::to_string(tStamp) + ".data";
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

		char *buffer = new char[len]();
//		std::cout<<"read value : "<<strlen(buffer)<<std::endl;
        in.read(buffer, len);      // 128 --> ??

		std::string str = buffer;
		std::cout<<"read file value : "<<str<<std::endl;

		in.close();
		if (strlen(buffer) != len) {     // a error occured
			std::cout<<"read value size : "<<strlen(buffer)<<std::endl;
			return str.substr(0, len-1);
		}
		return str;
	}

	Entry* BcFiles::writeBcFile(BcFile *bf, const std::string& key, const std::string& value) {

		auto timestamp = getCurrentOfSecond();
		uint64_t ts = getCurrentOfMicroSecond();
		std::cout<<"t : "<<ts<<std::endl;
        uint32_t kSz = key.size();
		uint32_t valueSz = value.size();
	    auto keySize = std::to_string(key.size());
	    auto valueSize = std::to_string(value.size());
	    //auto hashKey = BKDRHash(key.c_str());

        // HeaderSize = 16  --> 20
	    uint64_t valueOffset = bf->file_offset + 20 + kSz;
	    std::string strData = std::to_string(ts) + keySize + valueSize + key + value;
        std::cout<<"offset : "<<valueOffset<<std::endl;

        char dataHeader[20];
		char* crc32 = getCrc32(strData.c_str(), strData.size());
//		std::cout<<"crc32 : "<<crc32<<std::endl;
		memcpy(dataHeader, crc32, 4);
		//EncodeDataHeader(dataHeader, ts, kSz, valueSz);
		dataHeader[4] = ts >> 56;
		dataHeader[5] = ts >> 48;
		dataHeader[6] = ts >> 40;
		dataHeader[7] = ts >> 32;
		dataHeader[8] = ts >> 24;
		dataHeader[9] = ts >> 16;
		dataHeader[10] = ts >> 8;
		dataHeader[11] = ts;
		dataHeader[12] = kSz >> 24;
		dataHeader[13] = kSz >> 16;
		dataHeader[14] = kSz >> 8;
		dataHeader[15] = kSz;
		dataHeader[16] = valueSz >> 24;
		dataHeader[17] = valueSz >> 16;
		dataHeader[18] = valueSz >> 8;
		dataHeader[19] = valueSz;

		//char *ch = "ch";
		//char ch[1000];
		//EncodeData(ch, crc32, strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), key, value); 
		
		//write(bf->fp, ch, std::to_string(ch).size());
        //strData = crc32 + strData;

		write(bf->fp, dataHeader, 20);
		write(bf->fp, key.c_str(), kSz);
		write(bf->fp, value.c_str(), valueSz);
		//std::cout<<"data : "<<ch<<std::endl;
		//write(bf->fp, crc32, 4);
		//write(bf->fp, strData.c_str(), strData.size());


        char hintHeader[24];
		//std::string strHint = timestamp + keySize + valueSize + std::to_string(valueOffset) + key;
		hintHeader[0] = ts >> 56;
		hintHeader[1] = ts >> 48;
		hintHeader[2] = ts >> 40;
		hintHeader[3] = ts >> 32;
		hintHeader[4] = ts >> 24;
		hintHeader[5] = ts >> 16;
		hintHeader[6] = ts >> 8;
		hintHeader[7] = ts;
		hintHeader[8] = kSz >> 24;
		hintHeader[9] = kSz >> 16;
		hintHeader[10] = kSz >> 8;
		hintHeader[11] = kSz;
		hintHeader[12] = valueSz >> 24;
		hintHeader[13] = valueSz >> 16;
		hintHeader[14] = valueSz >> 8;
		hintHeader[15] = valueSz;
		hintHeader[16] = valueOffset >> 56;
		hintHeader[17] = valueOffset >> 48;
		hintHeader[18] = valueOffset >> 40;
		hintHeader[19] = valueOffset >> 32;
		hintHeader[20] = valueOffset >> 24;
		hintHeader[21] = valueOffset >> 16;
		hintHeader[22] = valueOffset >> 8;
		hintHeader[23] = valueOffset;
		
		std::cout<<"ts : "<<ts<<std::endl;
		
		//char chHint[1000];
		//EncodeHint(chHint, strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), valueOffset, key);
		
		write(bf->hintFp, hintHeader, 24);
		write(bf->hintFp, key.c_str(), kSz);

        //HeaderSize --> 20
		bf->file_offset = bf->file_offset + 20 + kSz + valueSz;
        
		std::cout<<"write finishing "<<std::endl;
        return new Entry(bf->file_id, valueOffset, valueSz, ts);
	}

    // tombstone : keySize=0, valueSize=0
	void BcFiles::delBcFile(BcFile *bf, const std::string& key) {

		auto timestamp = getCurrentOfSecond();            // std::string
	    uint64_t ts = getCurrentOfMicroSecond();
		auto keySize = std::to_string(0);
	    auto valueSize = std::to_string(0);  // 0
	    //auto hashKey = BKDRHash(key.c_str());

        auto valueOffset = bf->file_offset + HeaderSize + strtoull(keySize.c_str(), NULL, 10);
	    std::string strData = timestamp + keySize + valueSize + key;
		//char *ch = getCrc32(strData.c_str(), strData.size());
		char* crc32 = getCrc32(strData.c_str(), strData.size());      // char*
        
		char ch[1000];
		EncodeData(ch, crc32, strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), key, NULL); 
		write(bf->fp, ch, sizeof(ch));
	   
	    //std::string strHint = timestamp + keySize + valueSize + std::to_string(valueOffset) + key;
		char chHint[1000];
		EncodeHint(chHint, strtoul(timestamp.c_str(), NULL, 10), strtoul(keySize.c_str(), NULL, 10), strtoul(valueSize.c_str(), NULL, 10), valueOffset, key);

		write(bf->hintFp, chHint, sizeof(chHint));
        
		// HeaderSize 16
		bf->file_offset = bf->file_offset + HeaderSize + strtoull(keySize.c_str(), NULL, 10) + strtoull(valueSize.c_str(), NULL, 10);
		
		return;
	}
   
    void BcFiles::setBFS(std::map<uint64_t, BcFile*> bfs) {
		this->bfs = bfs;
	}

	std::map<uint64_t, BcFile*> BcFiles::getBFS() {
		return this->bfs;
	}


