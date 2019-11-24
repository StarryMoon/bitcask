package main

import (
	"os"
        "strings"
        "time"
        "fmt"
	"github.com/bitcask"
)

func main() {
	os.RemoveAll("exampleBitcaskDir")
	bc, err := bitcask.Open("exampleBitcaskDir", nil)
	if err != nil {
		//logger.Fatal(err)
	}
	defer bc.Close()

        key := []byte("usutachina")
        value := []byte(strings.Repeat("x", 128))
        fmt.Println("starting... \n", time.Now().UnixNano() / 1e6)
        for i := 0; i < 2000000; i++ {
            bc.Put(key, value)
        }
        fmt.Println("ending... \n", time.Now().UnixNano() / 1e6)
//	k1 := []byte("xiaoMing")
//	v1 := []byte("毕业于新东方推土机学院")

//	k2 := []byte("zhanSan")
//	v2 := []byte("毕业于新东方厨师学院")

//	bc.Put(k1, v1)
//	bc.Put(k2, v2)

//	v1, _ = bc.Get(k1)
//	v2, _ = bc.Get(k2)
//	logger.Info(string(k1), string(v1))
//	logger.Info(string(k2), string(v2))
	// override
//	v2 = []byte("毕业于新东方美容美发学院")
//	bc.Put(k2, v2)
//	v2, _ = bc.Get(k2)
//	logger.Info(string(k2), string(v2))

//	bc.Del(k1)
//	bc.Del(k2)
//	logger.Info("毕业后的数据库：")
//	v1, e := bc.Get(k1)
//	if e != bitcask.ErrNotFound {
//		logger.Info(string(k1), "shoud be:", bitcask.ErrNotFound)
//	} else {
//		logger.Info(string(k1), "已经毕业.")
//	}
//	v2, e = bc.Get(k2)
//	if e != bitcask.ErrNotFound {
//		logger.Info(string(k1), "shoud be:", bitcask.ErrNotFound)
//	} else {
//		logger.Info(string(k2), "已经毕业.")
//	}

}
