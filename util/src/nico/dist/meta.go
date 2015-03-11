package nico

import (
	"bufio"
	"compress/gzip"
	"encoding/json"
	"io"
	"log"
	"os"
	"sort"
	"time"
)

type MetaInfo struct {
	VideoId     string    `json:"video_id"`
	Title       string    `json:"title"`
	ViewCounter int64     `json:"view_counter"`
	UploadTime  time.Time `json:"upload_time"`
	Tags        []TagItem
}
type TagItem struct {
	Tag string `json:"tag"`
	//	Lock     bool   `json:"lock"`
	//	Category bool   `json:"category"`
}

type TagCount struct {
	Tag   string
	Count int
}

type TagRank []TagCount

func (self TagRank) Len() int {
	return len(self)
}

func (self TagRank) Swap(i, j int) {
	self[i], self[j] = self[j], self[i]
}

func (self TagRank) Less(i, j int) bool {
	return self[i].Count > self[j].Count
}
func CountTags(metas []MetaInfo) map[string]int {
	dic := make(map[string]int)
	for _, meta := range metas {
		for _, tag := range meta.Tags {
			dic[tag.Tag]++
		}
	}
	return dic
}

func MakeRanking(info map[string]int) TagRank {
	rank := make([]TagCount, 0, len(info))
	for k, cnt := range info {
		rank = append(rank, TagCount{k, cnt})
	}
	sort.Sort(TagRank(rank))
	return rank
}

func PrintDataSet(fname string, metas []MetaInfo) {
	desc, err := os.Create(fname)
	defer desc.Close()
	if err != nil {
		panic(err)
	}
	gzw := gzip.NewWriter(desc)
	writer := bufio.NewWriter(gzw)
	for _, meta := range metas {
		dat, err := json.Marshal(&meta)
		if err != nil {
			panic(err)
		}
		_, err = writer.Write(dat)
		if err != nil {
			panic(err)
		}
		writer.Write([]byte{'\n'})
	}
	writer.Flush()
	gzw.Flush()
	gzw.Close()
}

func LoadDataSet(fname string, metas []MetaInfo, limit int64) []MetaInfo {
	desc, err := os.Open(fname)
	defer desc.Close()
	if err != nil {
		panic(err)
	}
	gzr, err := gzip.NewReader(desc)
	if err != nil {
		panic(err)
	}
	reader := bufio.NewReaderSize(gzr, 1024*1024)
	var bline []byte
	cnt := 0
	for bline, _, err = reader.ReadLine(); err == nil; bline, _, err = reader.ReadLine() {
		meta := MetaInfo{}
		e := json.Unmarshal(bline, &meta)
		if e != nil {
			log.Printf("[Warn] %v for %v", e, string(bline))
			continue
		}
		if meta.ViewCounter >= limit {
			metas = append(metas, meta)
			cnt++
		}
	}
	log.Printf("Loaded: %v, %v videos.", fname, cnt)
	if err != io.EOF {
		panic(err)
	}
	return metas
}
