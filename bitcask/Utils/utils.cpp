#include "utils.h"
#include <chrono>
#include <stdio.h>
#include "timeRela.h"
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include "../bitcask.h"
#include "../bcFile.h"

int getRandomInt() {
	//errror
	//std::default_random_engine e(std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now())+static_cast<long long>((&temp))&65535 );


	std::default_random_engine e(clock());
	std::uniform_int_distribution<unsigned int> uig;
	return uig(e);

	//srand(std::chrono::high_resolution_clock::to_time_t(std::chrono::high_resolution_clock::now()));
	//return static_cast<int>(rand());
}

int getRandomIntRatio(int count, int overwriteRatio) {
    int base = 100 / overwriteRatio;
    
    if (count==0) {
        return count;
    }

    if (count % base == 0) {
        return count-1;
    } else {
        return count;
    }
}

std::string getRandStr(int length) {
    std::random_device rd; 
    std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dis (65, 122);
    std::string retStr;
    for (int i=0;i<length;i++){
        auto chaAscii=dis(gen);
        if(90<chaAscii&&chaAscii<97)
            chaAscii+=7;
        retStr.push_back(static_cast<char>(chaAscii));
    }

    return retStr;
}

char* getCrc32(const char* InStr, int len) {
  
    std::cout<<"crc32"<<std::endl;
    unsigned int Crc32Table[256];    
    int i,j;      
    unsigned int Crc;      
    for (i = 0; i < 256; i++){      
        Crc = i;      
        for (j = 0; j < 8; j++){      
            if (Crc & 1) {    
                Crc = (Crc >> 1) ^ 0xEDB88320; 
            } else {   
                Crc >>= 1;
            }   
        }      
        Crc32Table[i] = Crc;      
    }      
     
    Crc=0xffffffff; 

    for(int i=0; i<len; i++) {        
        Crc = (Crc >> 8) ^ Crc32Table[(Crc & 0xFF) ^ InStr[i]];      
    }   
     
    Crc ^= 0xFFFFFFFF;  

    char buf[4];
    buf[0] = Crc >> 24;
    buf[1] = Crc >> 16;
    buf[2] = Crc >> 8;
    buf[3] = Crc;

    //uint32_t crc32 = strtoul(buf, NULL, 10);
    return buf;    
}

void createWriteableFile(Bitcask *bc) {
    std::string file_id = getCurrentOfSecond();
    auto file_name = bc->getDirName() + "/" + file_id + ".data";
    int fd;
    fd = open(file_name.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
    if (fd == -1) {
        throw std::runtime_error("data file open error.");
    }
    bc->setActiveFile_fp(fd);
    bc->setActiveFile_fileId(strtoul(file_id.c_str(), NULL, 10));  // string -> uint32_t
  
    return;
}

void createHintFile(Bitcask *bc) {
    auto file_name = bc->getDirName() + "/" + std::to_string(bc->getActiveFile_fileId()) + ".hint";
    int fd;
    fd = open(file_name.c_str(), O_CREAT|O_WRONLY|O_APPEND, S_IRUSR);
    if (fd == -1) {
        throw std::runtime_error("hint file open error.");
    }

    bc->setActiveFile_hintFp(fd);
    
    return;
}

void checkActiveFile(Bitcask *bc) {
    uint64_t offset = bc->getActiveFile_offset();
    auto logSize = bc->getLogSize();
    std::cout<<"check"<<std::endl;
    if (offset >= logSize) {
        std::cout<<"offseeet : "<<offset<<std::endl;
        close(bc->getActiveFile_fp());
        close(bc->getActiveFile_hintFp());
        
        std::cout<<"put bcfiles"<<std::endl;
        // put
        bc->getBCF()->put_BcFiles(bc->getActiveFile(), bc->getActiveFile_fileId());

        // data file
        //bc->getActiveFile()->fp = createWriteableFile(bc);
        createWriteableFile(bc);

        // hint file
        // bc->activeFile->hintFp = createHintFile(bc);
        createHintFile(bc);
        bc->setActiveFile_offset(0);   
    }
}

std::vector<std::string>* scanHintFiles() {
    std::cout<<"scanHIntFiles"<<std::endl;
    struct dirent *ptr;    
    DIR *dir;
    std::string PATH = "./bitcaskTest";
    dir = opendir(PATH.c_str());
    std::vector<std::string> files;

    while((ptr=readdir(dir))!=NULL) {
 
        //skip '.'/'..'
        if(ptr->d_name[0] == '.')
            continue;
        //cout << ptr->d_name << endl;
        std::string str = ".hint";
        const char *show;
        show = strstr(std::string(ptr->d_name).c_str(), str.c_str());
        if(show != NULL) {
            files.push_back(ptr->d_name);
        }  
    }
    
    for (int i = 0; i < files.size(); ++i)
    {
        std::cout << files[i] << std::endl;
    }
 
    closedir(dir);

	return &files;
 
  return NULL; 
}

std::vector<std::string>* listDataFiles() {
	struct dirent *ptr;    
    DIR *dir;
    std::string PATH = "./bitcaskTest";
    dir = opendir(PATH.c_str()); 
    std::vector<std::string> files;
/*
    while((ptr=readdir(dir))!=NULL) {
 
        //skip '.'/'..'
        if(ptr->d_name[0] == '.')
            continue;
        //cout << ptr->d_name << endl;
        std::string str = ".data";
        const char *show;
        show = strstr(std::string(ptr->d_name).c_str(),str.c_str());
        if(show != NULL) {
            files.push_back(ptr->d_name);
        }  
    }
    
    for (int i = 0; i < files.size(); ++i)
    {
        std::cout << files[i] << std::endl;
    }
 
    closedir(dir);

    // sort ???
*/
	return &files;
}

uint32_t getLastFileInfo(std::vector<std::string> *existHintFiles) {
    // get the timestamp of last file
    uint32_t lastStamp = 0;
/*    std::vector<std::string>::iterator iter;
	  for (iter=existHintFiles->begin(); iter!=existHintFiles->end(); iter++) {
        std::string str = *iter;
        int firstPos = str.find('.', 0);
        int secondPos = str.rfind('.', str.length()-1);
        std::string tstampStr = str.substr(firstPos, secondPos);
        uint32_t tstamp = strtoul(tstampStr.c_str(), NULL, 10);
        if (lastStamp < tstamp) {
          lastStamp = tstamp;
        }
    }
*/
    return lastStamp;
}

// lockfile   O_CREAT|O_EXCL
int lockFile(std::string path) {
    int fd;
    fd = open(path.c_str(), O_CREAT|O_EXCL|O_RDWR, S_IRUSR);
    return fd;
}

void writePID(int fd, uint32_t file_id) {
    std::string str = std::to_string((int)getpid());
    str = str + "\t" + std::to_string(file_id) + ".data";
    const char *ch = str.c_str();

    write(fd, ch, str.size());
}