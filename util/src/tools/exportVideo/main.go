package main

import (
	"database/sql"
	"encoding/binary"
	"fmt"
	"log"
	opener "nico/db"
	"nico/dist"
	"runtime"
	"strings"
	"time"
)

func tags2bin(tag2int map[string]uint32, tags []nico.TagItem) string {
	var buff = make([]byte, 4*len(tags))
	for i := range tags {
		tag := tags[i].Tag
		id := tag2int[tag]
		binary.LittleEndian.PutUint32(buff[i*4:], (id))
	}
	var str = make([]string, 0)
	for _, b := range buff {
		str = append(str, fmt.Sprintf("%02X", b))
	}
	return strings.Join(str, "")
}

func findAllTagIndex(db *sql.DB, tags []string) map[string]uint32 {
	dic := make(map[string]uint32)
	{
		selStmt, err := db.Prepare("select `id` from `tags` where `name` LIKE ?")
		if err != nil {
			panic(err)
		}
		defer selStmt.Close()
		var id uint32
		for _, tag := range tags {
			r := strings.Replace(tag, `\`, `\\`, -1)
			r = strings.Replace(r, `%`, `\%`, -1)
			r = strings.Replace(r, `_`, `\_`, -1)
			err = selStmt.QueryRow(r).Scan(&id)
			if err == sql.ErrNoRows {
				log.Printf("Not Found: %v", tag)
				panic(err)
			} else if err == nil {
				dic[tag] = id
			} else {
				panic(err)
			}
		}
	}
	return dic
}
func enumAllTags(metas []nico.MetaInfo) []string {
	tagbs := make(map[string]bool)
	tagcs := make(map[string]bool)
	for _, meta := range metas {
		for _, tag := range meta.Tags {
			l := strings.ToLower(tag.Tag)
			if !tagcs[l] {
				tagcs[l] = true
				tagbs[tag.Tag] = true
			}
		}
	}
	tags := make([]string, 0, len(tagbs))
	for k, _ := range tagbs {
		tags = append(tags, k)
	}
	return tags
}

const PARA = 10

func main() {
	runtime.GOMAXPROCS(runtime.NumCPU())
	db := opener.Connect("localhost")
	defer db.Close()
	log.Printf("Connected.")
	const createdFormat = "2006-01-02 15:04:05"
	for i := 0; i <= 1925; i++ {
		metas := make([]nico.MetaInfo, 0)
		metas = nico.LoadDataSet(fmt.Sprintf("/opt/dolce/cache/data/meta/%04d.dat.gz", i), metas, 0)
		tags := findAllTagIndex(db, enumAllTags(metas))
		log.Printf("Dict loaded")
		for _, meta := range metas {
			tagb := tags2bin(tags, meta.Tags)
			if len(tagb)%4 != 0 && len(tagb) > 40 {
				panic(fmt.Sprintf("Tagb Length: %v", len(tagb)))
			}
			if len(meta.VideoId) <= 0 {
				panic(fmt.Sprintf("meta.VidoId Empty: %v", meta))
			}
			fmt.Printf("%s\t%d\t%s\t%s\n", meta.VideoId, meta.ViewCounter, time.Unix(meta.UploadTime.Unix(), 0).Format(createdFormat), tags2bin(tags, meta.Tags))
		}
	}

}
