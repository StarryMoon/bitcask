#pragma once
#pragma comment(lib, "pthreadVC2.lib") 

#include <vector>
#include <map>
#include <fstream>
#include <atomic>
#include <string>
#include <pthread.h>
#include <dirent.h>
#include "entry.h"
#include "bcFile.h"
#include "Utils/coding.h"

class Bitcask
{
public:

    Bitcask();
	~Bitcask();
	
    void setBCF(BcFiles *bcf);
	BcFiles* getBCF();

    void setActiveFile(BcFile *activeFile);
	BcFile* getActiveFile();
	int getActiveFile_fp();
    void setActiveFile_fp(int fp);
	uint32_t getActiveFile_fileId();
    void setActiveFile_fileId(uint32_t file_id);
	uint64_t getActiveFile_offset();
	void setActiveFile_offset(uint64_t offset);
    int getActiveFile_hintFp();
    void setActiveFile_hintFp(int hintFp);

	void setDirName(std::string dirName);
	std::string getDirName();

	uint64_t getLogSize(); 	

    std::string getTestPath();

	void setLocker(int fd);
	int getLocker();

	std::string get(std::string key);
	void put(const std::string& key, const std::string& value);
	void del(std::string key);

    void fold(void (*f)(const std::string& key));
	void merge();

    void parseHintFiles(std::vector<std::string>*);

    void scanEntry(std::vector<std::pair<std::string, Entry*>>, std::vector<std::string>*);

	BcFile* getFileState(uint32_t file_id);

private:

    //  std::atomic_flag writeFlag = ATOMIC_FLAG_INIT;

    const std::string lockFileName = "bitcask.lock";    // path of lock
	const uint64_t LogSize = 10*1024*1024;      // 10M
	const std::string testPath = "bitcaskTest"; // xxxxx.data  xxxxx.hint
    BcFiles *bcf;         // readonly files
	BcFile *activeFile;   // data/hint file
	std::string dirName;  // test dir
	int locker;                  // dir lock
	pthread_rwlock_t rwlock;    // read-write lock

    // map: insert --> return directly/not overlap
	// [key] = value  --> overlap
	std::map<std::string, Entry*> hashTable;   
	
	// vector   key->file_info   duplicate items exist
};
