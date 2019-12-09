#include  "coding.h"
#include <iostream>

/*
void EncodeData(char* buf, const std::string& key, const std::string& value) {
  memcpy(buf, key.data(), key.size());
  memcpy(buf+key.size(), value.data(), value.size());
  return;
}

void DecodeData(char* buf, uint32_t key_size, uint32_t value_size, std::string* value) {
  *value = std::string(buf+key_size, value_size);
  return;
}
*/


void EncodeDataHeader(char* buf, uint64_t tStamp, uint32_t kSz, uint32_t valueSz) {
  /*  buf[4] = tStamp >> 56;
    buf[5] = tStamp >> 48;
    buf[6] = tStamp >> 40;
    buf[7] = tStamp >> 32;
    buf[8] = tStamp >> 24;
    buf[9] = tStamp >> 16;
    buf[10] = tStamp >> 8;
    buf[11] = tStamp;
    buf[12] = kSz >> 24;
    buf[13] = kSz >> 16;
    buf[14] = kSz >> 8;
    buf[15] = kSz;
    buf[16] = valueSz >> 24;
    buf[17] = valueSz >> 16;
    buf[18] = valueSz >> 8;
    buf[19] = valueSz;
    int pos =4;
    memcpy(buf+pos, EncodeFixed32(tStamp), 4);
    pos += 4;

    memcpy(buf+pos, EncodeFixed32(kSz), 4);
    pos += 4;

    memcpy(buf+pos, EncodeFixed32(valueSz), 4);
    pos += 4;
   */ 
}

void EncodeHintHeader(char* buf, uint32_t tStamp, uint32_t kSz, uint32_t valueSz, uint64_t fileOffset) {
    
    int pos =0;
    memcpy(buf+pos, EncodeFixed32(tStamp), 4);
    pos += 4;

    memcpy(buf+pos, EncodeFixed32(kSz), 4);
    pos += 4;

    memcpy(buf+pos, EncodeFixed32(valueSz), 4);
    pos += 4;

    memcpy(buf+pos, EncodeFixed64(fileOffset), 4);
}


void EncodeData(char *buf,
		 char *crc32,
         uint32_t tStamp,
		 uint32_t key_size,
		 uint32_t value_size,
         const std::string& key,
         const std::string& value) {
//  assert(buf);

  int pos = 0;

  memcpy(buf, crc32, 4);
  pos += sizeof(uint32_t);

  memcpy(buf+pos, EncodeFixed32(tStamp), 4);
  pos += sizeof(uint32_t);

  memcpy(buf+pos, EncodeFixed32(key_size), 4);
  pos += sizeof(uint32_t);

  memcpy(buf+pos, EncodeFixed32(value_size), 4);
  pos += sizeof(uint32_t);

  memcpy(buf+pos, key.data(), key_size);
  pos += key_size;

  memcpy(buf+pos, value.data(), value_size);

  return;
}

void DecodeData(char* buf,
                 uint32_t* crc32,
                 uint32_t* tStamp,
                 uint32_t* key_size,
                 uint32_t* value_size,
                 std::string* key,
                 std::string* value) {
  size_t pos = 0;

  *crc32 = DecodeFixed32(buf);
  pos += sizeof(uint32_t);

  *tStamp = DecodeFixed32(buf+pos);
  pos += sizeof(uint32_t);

  *key_size = DecodeFixed32(buf+pos);
  pos += sizeof(uint32_t);

  *value_size = DecodeFixed32(buf+pos);
  pos += sizeof(uint32_t);

  *key = std::string(buf+pos, *key_size);
  pos += *key_size;

  *value = std::string(buf+pos, *value_size);

  return;
}

char* EncodeFixed32(uint32_t num) {
    char buf[4];
    buf[0] = num >> 24;     
    buf[1] = num >> 16;
    buf[2] = num >> 8;
    buf[3] = num;
    return buf;
}

char* EncodeFixed64(uint64_t num) {
    char buf[8];

    buf[0] = num >> 56;     
    buf[1] = num >> 48;
    buf[2] = num >> 40;
    buf[3] = num >> 32;
    buf[4] = num >> 24;     
    buf[5] = num >> 16;
    buf[6] = num >> 8;
    buf[7] = num;

    return buf;
}

uint32_t DecodeFixed32(const char* ptr) {
  uint32_t result;
  std::memcpy(&result, ptr, sizeof(result));
  return result;
}

uint64_t DecodeFixed64(const char* ptr) {
  uint64_t result;
  std::memcpy(&result, ptr, sizeof(result));
  return result;
}


void EncodeHint(char* buf, uint32_t tStamp, uint32_t ksz, uint32_t valueSz, uint64_t file_offset, const std::string& key) {
  
  int pos = 0;
  std::memcpy(buf, EncodeFixed32(tStamp), 4);
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, EncodeFixed32(ksz), 4);
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, EncodeFixed32(valueSz), 4);
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, EncodeFixed64(file_offset), 8);
  pos += sizeof(uint64_t);
  
  std::memcpy(buf+pos, key.data(), ksz);
  
  return;
}

void DecodeHint(char* buf, uint32_t* tStamp, uint32_t* ksz, uint32_t* valueSz, uint64_t* file_offset, std::string* key) {
  size_t pos = 0;
  *tStamp = DecodeFixed32(buf);
  pos += sizeof(uint32_t);

  *ksz = DecodeFixed32(buf+pos);
  pos += sizeof(uint32_t);

  *valueSz = DecodeFixed32(buf+pos); 
  pos += sizeof(uint32_t);

  *file_offset = DecodeFixed64(buf+pos);
  pos += sizeof(uint64_t);

  *key = std::string(buf+pos, *ksz);
}


void DecodeHintHeader(char* buf, uint32_t* tStamp, uint32_t* ksz, uint32_t* valueSz, uint64_t* file_offset) {
  std::cout<<"file headerrrr : "<<buf<<std::endl; 
  size_t pos = 0;
  *tStamp = DecodeFixed32(buf);
  pos += sizeof(uint32_t);
  std::cout<<"file header : "<<*tStamp<<std::endl; 

  std::cout<<"file header : "<<buf+pos<<std::endl; 
  *ksz = DecodeFixed32(buf+pos);
  pos += sizeof(uint32_t);
  std::cout<<"file header : "<<*ksz<<std::endl; 

  *valueSz = DecodeFixed32(buf+pos); 
  pos += sizeof(uint32_t);
  std::cout<<"file header : "<<*valueSz<<std::endl; 

  *file_offset = DecodeFixed64(buf+pos);
}


