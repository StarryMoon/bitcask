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

void EncodeData(char *buf,
		 uint32_t crc32,
         uint32_t tStamp,
		 uint32_t key_size,
		 uint32_t value_size,
         const std::string& key,
         const std::string& value) {
//  assert(buf);

  size_t pos = 0;
 
//std::cout<<"encode"<<std::endl;

  std::memcpy(buf, &crc32, sizeof(uint32_t));
  pos += sizeof(uint32_t);
 //std::cout<<"encode1"<<std::endl;
  std::memcpy(buf+pos, &tStamp, sizeof(uint32_t));
  pos += sizeof(uint32_t);
//std::cout<<"encode2"<<std::endl;
  std::memcpy(buf+pos, &key_size, sizeof(uint32_t));
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, &value_size, sizeof(uint32_t));
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, key.data(), key_size);
  pos += key_size;

  std::memcpy(buf+pos, value.data(), value_size);

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
    
  size_t pos = 0;
  std::memcpy(buf, &tStamp, sizeof(uint32_t));
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, &ksz, sizeof(uint32_t));
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, &valueSz, sizeof(uint32_t));
  pos += sizeof(uint32_t);

  std::memcpy(buf+pos, &file_offset, sizeof(uint64_t));
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
    
  size_t pos = 0;
  *tStamp = DecodeFixed32(buf);
  pos += sizeof(uint32_t);

  *ksz = DecodeFixed32(buf+pos);
  pos += sizeof(uint32_t);

  *valueSz = DecodeFixed32(buf+pos); 
  pos += sizeof(uint32_t);

  *file_offset = DecodeFixed64(buf+pos);
}


