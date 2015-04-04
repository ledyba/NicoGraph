package file

import (
	"bufio"
	"encoding/binary"
	"io"
	"log"
	"nico/dist"
	"os"
	"sort"
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
	tagDict  map[string]uint32
	Videos   []Video
}

func NewDB(tags string) *DB {
	db := &DB{}
	db.Videos = make([]Video, 0)
	db.Tags, db.tagDict = enumTags(tags)
	return db
}

func enumTags(fname string) ([]string, map[string]uint32) {
	tags := make(map[string]uint32, 0)
	tagl := make([]string, 0)
	f, err := os.Open(fname)
	if err != nil {
		panic(err)
	}
	defer f.Close()
	bf := bufio.NewReader(f)
	var bline []byte
	cnt := uint32(0)
	for bline, _, err = bf.ReadLine(); err == nil; bline, _, err = bf.ReadLine() {
		t := string(bline)
		tags[t] = cnt
		tagl = append(tagl, t)
		cnt++
	}
	if err != io.EOF {
		panic(err)
	}
	log.Printf("Loaded: %d tags", cnt)
	return tagl, tags
}

func (db *DB) Serialize(tagf *bufio.Writer, vf *bufio.Writer, linkf *bufio.Writer) {
	sort.Sort(VideoList(db.Videos))
	for _, tag := range db.Tags {
		bs := []byte(tag)
		tagf.WriteByte(byte(len(bs)))
		tagf.Write(bs)
	}
	for _, vid := range db.VideoIds {
		bs := []byte(vid)
		vf.WriteByte(byte(len(bs)))
		vf.Write(bs)
	}
	for i := range db.Videos {
		video := &db.Videos[i]
		binary.Write(linkf, binary.LittleEndian, video.VideoId)
		binary.Write(linkf, binary.LittleEndian, video.ViewCount)
		binary.Write(linkf, binary.LittleEndian, video.UploadedAt)
		for _, tag := range video.Tags {
			binary.Write(linkf, binary.LittleEndian, tag)
		}
	}
}

type VideoList []Video

func (vs VideoList) Len() int {
	return len(vs)
}

func (vs VideoList) Swap(i, j int) {
	vs[i], vs[j] = vs[j], vs[i]
}

func (vs VideoList) Less(i, j int) bool {
	return vs[i].UploadedAt < vs[j].UploadedAt
}
func (db *DB) searchTag(tag string) (uint32, bool) {
	n, ok := db.tagDict[tag]
	return n, ok
}

func (db *DB) AddVideo(info *nico.MetaInfo) {
	var tags [10]int32
	for i := range tags {
		tags[i] = -1
	}
	fill := 0
	for i := range tags {
		if i < len(info.Tags) {
			if t, ok := db.searchTag(info.Tags[i].Tag); ok {
				tags[fill] = int32(t)
				fill++
			}
		}
	}
	if fill < 2 {
		return
	}
	vid := len(db.VideoIds)
	db.VideoIds = append(db.VideoIds, info.VideoId)
	db.Videos = append(db.Videos, Video{uint32(vid), uint32(info.ViewCounter), uint64(info.UploadTime.Unix()), tags})
}
