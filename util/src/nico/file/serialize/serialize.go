package file

import (
	"bufio"
	"encoding/binary"
	"nico/dist"
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

func NewDB() *DB {
	db := &DB{}
	db.Tags = make([]string, 0)
	db.Videos = make([]Video, 0)
	db.tagDict = make(map[string]uint32, 0)
	return db
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
func (db *DB) searchTag(tag string) uint32 {
	n, ok := db.tagDict[tag]
	if ok {
		return n
	}
	n = uint32(len(db.Tags))
	db.tagDict[tag] = n
	db.Tags = append(db.Tags, tag)
	return n
}

func (db *DB) AddVideo(info *nico.MetaInfo) {
	var tags [10]int32
	for i := range tags {
		if i < len(info.Tags) {
			tags[i] = int32(db.searchTag(info.Tags[i].Tag))
		} else {
			tags[i] = -1
		}
	}
	vid := len(db.VideoIds)
	db.VideoIds = append(db.VideoIds, info.VideoId)
	db.Videos = append(db.Videos, Video{uint32(vid), uint32(info.ViewCounter), uint64(info.UploadTime.Unix()), tags})
}
