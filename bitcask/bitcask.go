package bitcask

import (
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
	"sync"
)

var (
	ErrNotFound = fmt.Errorf("Error: Not Found.")
	ErrIsNotDir = fmt.Errorf("Error: the file is not dir.")
)

func Open(dirName string, opts *Options) (*BitCask, error) {
	if opts == nil {
		opts1 := NewOptions(0, 0, -1, 60, true)
		opts = &opts1
	}

	_, err := os.Stat(dirName)
	if err != nil && !os.IsNotExist(err) {
		return nil, err
	}

	if os.IsNotExist(err) {
		err = os.Mkdir(dirName, 0755)
		if err != nil {
			return nil, err
		}
	}

	b := &BitCask{
		Opts:    opts,
		dirFile: dirName,
		oldFile: newBFiles(),
		rwLock:  &sync.RWMutex{},
	}

	// lock file
	b.lockFile, err = lockFile(dirName + "/" + lockFileName)
	if err != nil {
		return nil, err
	}

	b.keyDirs = NewKeyDir(dirName)

	// scan hint file
	files, _ := b.readableFiles()
	b.parseHint(files)

	// get the last fileid
	fileID, hintFp := lastFileInfo(files)

	var writeFp *os.File
	writeFp, fileID = setWriteableFile(fileID, dirName)

	hintFp = setHintFile(fileID, dirName)
	// close other hint
	closeReadHintFp(files, fileID)
	// setting writeable file, only one
	dataStat, _ := writeFp.Stat()
	bf := &BFile{
		fp:          writeFp,
		fileID:      fileID,
		writeOffset: uint64(dataStat.Size()),
		hintFp:      hintFp,
	}
	b.writeFile = bf

	// save the active data file name into bitcask.lock file
	writePID(b.lockFile, fileID)
	return b, nil
}

// BitCask 
type BitCask struct {
	Opts      *Options      // opts for bitcask
	oldFile   *BFiles       // hint file, data file
	lockFile  *os.File      // lock storage dir
	keyDirs   *KeyDirs      // hashMap in memory
	dirFile   string        // bitcask storage dir
	writeFile *BFile        // writeable files(active data/hint file )
	rwLock    *sync.RWMutex // rwlocker for bitcask Get and Put Operation
}

// Close opening fp
func (bc *BitCask) Close() {
	// close ActiveFiles
	bc.oldFile.close()
	// close writeable file
	bc.writeFile.fp.Close()
	bc.writeFile.hintFp.Close()
	// close lockFile
	bc.lockFile.Close()
	// delete lockFile
	os.Remove(bc.dirFile + "/" + lockFileName)
}

// Put
func (bc *BitCask) Put(key []byte, value []byte) error {
	bc.rwLock.Lock()
	defer bc.rwLock.Unlock()
	checkWriteableFile(bc)
	// write data into writeable file
	e, err := bc.writeFile.writeDatat(key, value)
	if err != nil {
		bc.rwLock.Unlock()
		return err
	}
	// add key/value into keydirs
	keyDirs.put(string(key), &e)
	return nil
}

// Get
func (bc *BitCask) Get(key []byte) ([]byte, error) {
//	bc.rwLock.RLock()
	e := keyDirs.get(string(key))
	if e == nil {
		return nil, ErrNotFound
	}

	fileID := e.fileID
	bf, err := bc.getFileState(fileID)
	if err != nil && os.IsNotExist(err) {
		//logger.Warn("key:", string(key), "=>the file is not exits:", fileID)
		//time.Sleep(time.Second)
		return nil, err
	}

//	val, err = bf.read(e.valueOffset, e.valueSz)
//  logger.Info("fileID", fileID, "entry offset:", e.valueOffset, "\t entryLen:", e.valueSz)
//	bc.rwLock.RUlock()
//	if err != nil {
//		return nil, err
//	}

	return bf.read(e.valueOffset, e.valueSz)
}

// Del  ==> tombstone : key=0, value=0
func (bc *BitCask) Del(key []byte) error {
	bc.rwLock.Lock()
	defer bc.rwLock.Unlock()
	if bc.writeFile == nil {
		return fmt.Errorf("Can Not Read The Bitcask Root Director")
	}
	e := keyDirs.get(string(key))
	if e == nil {
		return ErrNotFound
	}

	checkWriteableFile(bc)
	// write data into writeable file
	err := bc.writeFile.del(key)
	if err != nil {
		return err
	}
	// delete key/value from keydirs
	keyDirs.del(string(key))
	return nil
}

// return readable hint file: xxxx.hint
func (bc *BitCask) readableFiles() ([]*os.File, error) {
	filterFiles := []string{lockFileName}
	ldfs, err := listHintFiles(bc)
	if err != nil {
		return nil, err
	}

	fps := make([]*os.File, 0, len(ldfs))
	for _, filePath := range ldfs {
		if existsSuffixs(filterFiles, filePath) {
			continue
		}
		fp, err := os.OpenFile(bc.dirFile+"/"+filePath, os.O_RDONLY, 0755)
		if err != nil {
			return nil, err
		}
		fps = append(fps, fp)
	}
	if len(fps) == 0 {
		return nil, nil
	}
	return fps, nil
}

func (bc *BitCask) getFileState(fileID uint32) (*BFile, error) {
	// lock up it from writeable file
	if fileID == bc.writeFile.fileID {
		return bc.writeFile, nil
	}
	// if not exits in writeable file, look up it from OldFile
	bf := bc.oldFile.get(fileID)
	if bf != nil {
		return bf, nil
	}

	bf, err := openBFile(bc.dirFile, int(fileID))
	if err != nil {
		return nil, err
	}
	bc.oldFile.put(bf, fileID)
	return bf, nil
}

func (bc *BitCask) parseHint(hintFps []*os.File) {
	b := make([]byte, HintHeaderSize, HintHeaderSize)
	for _, fp := range hintFps {
		offset := int64(0)
		hintName := fp.Name()
		s := strings.LastIndex(hintName, "/") + 1
		e := strings.LastIndex(hintName, ".hint")
		fileID, _ := strconv.ParseInt(hintName[s:e], 10, 32)

		for {
			// parse hint header
			n, err := fp.ReadAt(b, offset)
			offset += int64(n)
			if err != nil && err != io.EOF {
				panic(err)
			}
			if err == io.EOF {
				break
			}

			if n != HintHeaderSize {
				panic(n)
			}

			tStamp, ksz, valueSz, valuePos := DecodeHint(b)
			//logger.Info("ksz:", ksz, "offset:", offset)
			if ksz+valueSz == 0 { // the record is deleted
				continue
			}

			// parse hint key
			keyByte := make([]byte, ksz)
			n, err = fp.ReadAt(keyByte, offset)
			if err != nil && err != io.EOF {
				panic(err)
			}
			if err == io.EOF {
				break
			}
			if n != int(ksz) {
				panic(n)
			}
			key := string(keyByte)

			e := &entry{
				fileID:      uint32(fileID),
				valueSz:     valueSz,
				valueOffset: valuePos,
				timeStamp:   tStamp,
			}
			offset += int64(ksz)
			// put entry into keyDirs
			keyDirs.put(key, e)
		}
	}
}

func (bc *Bitcask) fold(f func(key []byte) error) (err error) {
    bc.rwLock.Lock()
	defer bc.rwLock.Unlock()

	bc.keyDirs.ForEach(func(node ) bool{
		if err = f(node.Key()); err != nil {
			return false
		}

		return true
	})
	
	return
}