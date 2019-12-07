#pragma once

#include <string>
//#include <string.h>

// data file
/**
	    crc32	:	tStamp	:	ksz	:	valueSz	:	key	:	value
	    4 		:	4 		: 	4 	: 		4	:	xxxx	: xxxx
**/

//hint file
/**
		tStamp	:	ksz	:	valueSz	:	valuePos	:	key
	    4       :   4   :   4       :       8       :   xxxxx
**/

//void EncodeData(char* buf, const std::string& key, const std::string& value);

//void DecodeValue(char* buf, uint32_t key_size, uint32_t value_size, std::string* value);

void EncodeData(char* buf, uint32_t crc32, uint32_t tStamp, uint32_t key_size, uint32_t value_size, const std::string& key, const std::string& value);

void DecodeData(char* buf, uint32_t* crc32, uint32_t* tStamp, uint32_t* key_size, uint32_t* value_size, const std::string& key, const std::string& value);

uint32_t DecodeFixed32(const char* buf);

uint64_t DecodeFixed64(const char* buf);

void EncodeHint(char* buf, uint32_t tStamp, uint32_t ksz, uint32_t valuzeSz, uint64_t file_offset, const std::string& key);
void DecodeHint(char* buf, uint32_t* tStamp, uint32_t* ksz, uint32_t* valueSz, uint64_t* file_offset, const std::string& key);

void DecodeHintHeader(char* buf, uint32_t* tStamp, uint32_t* ksz, uint32_t* valueSz, uint64_t* file_offset);
