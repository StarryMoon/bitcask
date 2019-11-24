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

        key := []byte("usutachina")                  // 10 bytes
        value := []byte(strings.Repeat("x", 128))    // 128 bytes
        fmt.Println("starting... \n", time.Now().UnixNano() / 1e6)
        for i := 0; i < 2000000; i++ {
            bc.Put(key, value)
        }
        fmt.Println("ending... \n", time.Now().UnixNano() / 1e6)




//	v1, _ = bc.Get(k1)
//	v2, _ = bc.Get(k2)

	// override
//	v2 = []byte("hair")
//	bc.Put(k2, v2)
//	v2, _ = bc.Get(k2)

//	bc.Del(k1)
//	logger.Info("db：")
//	v1, e := bc.Get(k1)
//	if e != bitcask.ErrNotFound {
//		logger.Info(string(k1), "shoud be:", bitcask.ErrNotFound)
//	} else {
//		logger.Info(string(k1), "finish.")
//	}

}


/*
func conCurrentPut() {
        f := func(wg *sync.WaitGroup, x int) {
                defer func() {
                        wg.Done()
                }()

                for i:= 0; i<=100; i++ {
                        if i%x == 0 {
                                key := []byte(fmt.Sprintf("k%d", i))
                                value := []byte(fmt.Sprintf("v%d", i))
                                bc.Put(key, value)
                        }
                }
        }

        wg.&sync.WaitGroup()

        go f(wg, 2)
        go f(wg, 3)
        go wg(wg, 5)
        wg.Add(3)

        wg.Wait()
}
*/