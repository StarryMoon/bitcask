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
#include <stdlib.h>
#include "Utils/coding.h"
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

struct BcFile {
	int fp;
	uint32_t file_id;
	uint64_t file_offset;
	int hintFp;
};

class BcFiles 
{
public:
    BcFiles();

	~BcFiles();
 
    BcFile* get_BcFiles(uint32_t file_id);

	void put_BcFiles(BcFile *bcf, uint32_t file_id);

	void close_BcFiles();

	void newBcFile(BcFile* bcf, std::ofstream fp, uint32_t file_id, uint64_t file_offset, std::ofstream hintFp); 

	void openBcFile(BcFile* bcf, std::string dirName, std::string tStamp);

	std::string readBcFile(BcFile *bf, std::string dirName, uint64_t offset, uint32_t len);

	Entry* writeBcFile(BcFile *bf, const std::string& key, const std::string& value);

    // tombstone : valuesize=0
	void delBcFile(BcFile *bf, const std::string& key);

    void setBFS(std::map<uint32_t, BcFile*> bfs);

	std::map<uint32_t, BcFile*> getBFS();

private:
    std::map<uint32_t, BcFile*> bfs;  // file_timestamp -> .data / .hint
	
	pthread_rwlock_t rwlock;   // lock

	//crc32	:	tStamp	:	ksz	:	valueSz	:	key:value
	//4		:		4	:	4	:	4		:	xxxx:xxxx
    static const uint64_t HeaderSize = 16;
	
	//tstamp	:	ksz	:	valuesz	:	valuePos	:	key
	//4			:	4	:	4		:	8			:	xxxx
	static const uint64_t HintHeaderSize = 20;
};



