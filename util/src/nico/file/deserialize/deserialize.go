package file

import (
	"bufio"
	"encoding/binary"
	"io"
	"log"
	"os"
)

type Video struct {
	VideoId    uint32
	ViewCount  uint32
	UploadedAt uint64
	Tags       [10]int32
}
type DB struct {
	Tags     []string
	VideoIds []string
	Videos   []Video
}

func newDB() *DB {
	db := &DB{}
	db.Tags = make([]string, 0)
	db.Videos = make([]Video, 0)
	return db
}
func readString(fname string) []string {
	log.Printf("Loading: %v", fname)
	var err error
	rf, err := os.Open(fname)
	defer rf.Close()
	if err != nil {
		panic(nil)
	}
	f := bufio.NewReader(rf)
	var c byte
	var n int
	strs := make([]string, 0)
	for {
		c, err = f.ReadByte()
		if err != nil {
			break
		}
		bstr := make([]byte, int(c))
		pos := 0
		for pos < int(c) {
			n, err = f.Read(bstr[pos:])
			pos += n
			if err != nil {
				break
			}
		}
		if err != nil {
			break
		}
		strs = append(strs, string(bstr))
	}
	if err != io.EOF {
		panic(err)
	}
	return strs
}
func LoadDB(tagf, vf, linkf string) *DB {
	db := newDB()
	var err error
	db.Tags = readString(tagf)
	db.VideoIds = readString(vf)
	log.Printf("Loading: %v", linkf)
	rf, err := os.Open(linkf)
	defer rf.Close()
	if err != nil {
		panic(nil)
	}
	f := bufio.NewReader(rf)
	for {
		video := Video{}
		err = binary.Read(f, binary.LittleEndian, &video.VideoId)
		if err != nil {
			break
		}
		binary.Read(f, binary.LittleEndian, &video.ViewCount)
		binary.Read(f, binary.LittleEndian, &video.UploadedAt)
		for i := 0; i < 10; i++ {
			binary.Read(f, binary.LittleEndian, &video.Tags[i])
		}
		db.Videos = append(db.Videos, video)
	}
	if err != io.EOF {
		panic(err)
	}
	return db
}
