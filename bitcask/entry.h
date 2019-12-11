#pragma once

#include <string>

class Entry
{
    public:
	    
        Entry();

		Entry(uint64_t file_id, uint64_t file_offset, uint32_t value_size, uint64_t tstamp) {
            this->file_id = file_id;
            this->file_offset_ = file_offset;
            this->value_size_ = value_size;
            this->tstamp_ = tstamp;
        }

        ~Entry();

        uint64_t getFileId() {
            return file_id; 
        }

        void setFileId(uint64_t file_id) {
            this->file_id = file_id; 
        }

        uint64_t getFileOffset() {
            return file_offset_; 
        }

        void setFileOffset(uint64_t file_offset) {
            this->file_offset_ = file_offset; 
        }

        uint32_t getValueSize() { 
            return value_size_; 
        }

        void setValueSize(uint32_t value_size) { 
            this->value_size_ = value_size; 
        }

		uint64_t getTstamp() {
            return tstamp_; 
        }

        void setTstamp(uint64_t tstamp) {
            this->tstamp_ = tstamp; 
        }

        uint32_t getIndexSize() {
            return sizeof(uint32_t) * 3 + sizeof(uint64_t);
        }

        void SetDeleted() {
            this->value_size_ = 0;
        }
        
        bool isNewerThan(Entry *old) {
	        if (old->getTstamp() < this->getTstamp()) {
		        return true;
	        } else if (old->getTstamp() > this->getTstamp()) {
		        return false;
	        }

/*	        if old->getFileId() < e->getFileId() {
		        return true
	        } else if old->getFileId() > e->getFileId() {
		        return false
	        }

	        if old->getFileOffset() < e->getFileOffset() {
		        return true
	        } else if old->getFileOffset() > e->getFileOffset() {
		        return false
	        }
*/
	        return false;   // equal
        }

        bool isEqual(Entry *old) {
            if (old->getTstamp() == this->getTstamp()) {
		        return true;
            }

            return false;
        }


    private:
        // fileID   :   value_pos    :   value_sz   :tstamp
        //     4    :       8       :       4       :   4
        uint64_t file_id;
        uint64_t file_offset_;
        uint32_t value_size_;
        uint64_t tstamp_;
};

